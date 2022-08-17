//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "meeting_controller.h"

#include "signaling/signaling_client.h"
#include "signaling/signaling_client_configuration.h"
#include "signaling/default_signaling_dependencies.h"
#include "signaling/default_signaling_client_factory.h"
#include "utils/logging.h"
#include "video/fake_video_source.h"
#include "audio_video/local_video_configuration.h"
#include "data_message/data_message_to_send.h"

#include "utils/log_sink.h"
#include "meeting_controller_configuration.h"
#include "meeting_controller_dependencies.h"

#include "webrtc/api/create_peerconnection_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_decoder_factory.h"
#include "webrtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "webrtc/api/video_codecs/builtin_video_decoder_factory.h"
#include "webrtc/api/video_codecs/builtin_video_encoder_factory.h"
#include "webrtc/modules/audio_device/dummy/file_audio_device_factory.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/jsep.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/api/rtp_transceiver_direction.h"
#include "webrtc/api/rtc_error.h"
#include "webrtc/media/base/media_channel.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/api/media_stream_interface.h"
#include "webrtc/api/rtp_parameters.h"

#include <memory>
#include <regex>
#include <string>

using namespace chime;

std::unique_ptr<MeetingController> MeetingController::Create(MeetingControllerConfiguration configuration,
                                                             std::unique_ptr<SignalingClient> signaling_client,
                                                             SessionDescriptionObserver* session_description_observer) {

  MeetingControllerDependencies dependencies{};
  dependencies.signaling_client = std::move(signaling_client);
  dependencies.session_description_observer = session_description_observer;

  dependencies.signaling_thread = rtc::Thread::Create();
  dependencies.signaling_thread->SetName("signaling_sdk_demo_thread", nullptr);
  dependencies.signaling_thread->Start();

  dependencies.log_sink = std::make_unique<LogSink>();
  SetSignalingLogLevel(configuration.log_level);
  rtc::LogMessage::AddLogToStream(dependencies.log_sink.get(), rtc::LS_VERBOSE);

  webrtc::FileAudioDeviceFactory::SetFilenamesToUse(configuration.input_audio_filename.c_str(),
                                                    configuration.output_audio_filename.c_str());

  // To stay in the meeting, Chime's audio server requires a consistent stream of audio packets at all times.
  //   For more ways to send dummy audio see webrtc::TestAudioDeviceModule (omitted for brevity).
  dependencies.peer_connection_factory = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */, dependencies.signaling_thread.get(),
      nullptr /* default adm */, webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
      webrtc::CreateBuiltinVideoEncoderFactory(), webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
      nullptr /* audio_processing */);

  return std::make_unique<MeetingController>(configuration, std::move(dependencies));
}

MeetingController::MeetingController(MeetingControllerConfiguration configuration,
                                     MeetingControllerDependencies dependencies) {
  signaling_client_ = std::move(dependencies.signaling_client);
  signaling_thread_ = std::move(dependencies.signaling_thread);
  log_sink_ = std::move(dependencies.log_sink);
  peer_connection_factory_ = std::move(dependencies.peer_connection_factory);
  session_description_observer_ = dependencies.session_description_observer;
}

MeetingController::~MeetingController() {
  if (local_video_transceiver_) {
    local_video_transceiver_->StopStandard();
  }
  if (local_video_source_) {
    local_video_source_->Stop();
  }
  attendee_id_to_remote_video_transceivers_.clear();
  local_video_source_ = nullptr;
  local_video_transceiver_ = nullptr;
  peer_connection_factory_ = nullptr;
  session_description_observer_ = nullptr;
}

void MeetingController::Start() {
  signaling_client_->Stop();
  signaling_client_->Start();
  signaling_client_->Run();
}

void MeetingController::Stop() {
  if (peer_connection_) peer_connection_->Close();
  if (log_sink_) {
    rtc::LogMessage::RemoveLogToStream(log_sink_.get());
  }
  signaling_client_->Stop();
}

int MeetingController::TURNCandidateCount() {
  auto description = peer_connection_->pending_local_description() ? peer_connection_->pending_local_description()
                                                                   : peer_connection_->current_local_description();

  if (!description || description->number_of_mediasections() == 0) return 0;

  const webrtc::IceCandidateCollection* candidates = description->candidates(/* mediasection */ 0);
  int num_turn_candidates = 0;
  for (size_t i = 0; i < candidates->count(); i++) {
    if (candidates->at(i)->server_url().find("turn") != std::string::npos) {
      num_turn_candidates++;
    }
  }
  return num_turn_candidates;
}

void MeetingController::SendUpdates() {
  RTC_LOG(LS_INFO) << "Attempting to set and send updates.";
  if (TURNCandidateCount() < 1) {
    RTC_LOG(LS_WARNING) << "No TURN candidates. Aborting set and send of updates.";
    return;
  }
  if (updates_in_flight_) {
    RTC_LOG(LS_WARNING) << "Updates are in flight. Aborting set and send of updates.";
    return;
  }
  updates_in_flight_ = true;

  const webrtc::SessionDescriptionInterface* local_desc = peer_connection_->pending_local_description();
  if (!local_desc) {
    RTC_LOG(LS_WARNING) << "Peer connection has no pending local description. "
                           "Aborting set and send of updates.";
    updates_in_flight_ = false;
    return;
  }

  std::string sdp_offer;
  local_desc->ToString(&sdp_offer);

  // Tell the backend that it is unified platform
  sdp_offer = std::regex_replace(sdp_offer, std::regex("o=-"), "o=mozilla-native");
  signaling_client_->SetLocalDescription(sdp_offer);
  RTC_LOG(LS_INFO) << "Updates set.";

  if (!signaling_client_->SendUpdates()) {
    RTC_LOG(LS_ERROR) << "Failed to send updates";
    updates_in_flight_ = false;
    return;
  }
  RTC_LOG(LS_INFO) << "Updates sent.";
}

bool MeetingController::IsValidTransceiver(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> in) {
  return in && in->direction() != webrtc::RtpTransceiverDirection::kStopped;
}

// Update the bitrate of send
void MeetingController::UpdateLocalSender() {
  rtc::scoped_refptr<webrtc::RtpSenderInterface> local_sender = local_video_transceiver_->sender();
  if (!local_sender) {
    RTC_LOG(LS_ERROR) << "No local_sender available";
    return;
  }
  webrtc::RtpParameters send_params = local_sender->GetParameters();

  for (size_t i = 0; i < send_params.encodings.size(); i++) {
    send_params.encodings[i].min_bitrate_bps = kMinkbps * 1000;
    send_params.encodings[i].max_bitrate_bps = kMaxkbps * 1000;
    send_params.encodings[i].active = true;
  }

  webrtc::RTCError err = local_sender->SetParameters(send_params);
  if (!err.ok()) RTC_LOG(LS_ERROR) << "Failed to send local sender's bitrates with error: " << err.message();
}

void MeetingController::StartLocalVideo() {
  if (!local_video_transceiver_ || !local_video_transceiver_->mid().has_value()) {
    RTC_LOG(LS_WARNING) << "No transceiver present ignoring...";
    return;
  }

  if (!local_video_source_) {
    local_video_source_ = new rtc::RefCountedObject<FakeVideoSource>();
  }

  // Send 480 X 320 box
  local_video_source_->Start(kVideoSendWidth, kVideoSendHeight);

  SetExternalVideoSource(local_video_source_, webrtc::VideoTrackInterface::ContentHint::kNone);
  UpdateLocalSender();
  if (IsValidTransceiver(local_video_transceiver_)) {
    RTC_LOG(LS_INFO) << "Setting local_video_transceiver direction to send";
    webrtc::RTCError err = local_video_transceiver_->SetDirectionWithError(webrtc::RtpTransceiverDirection::kSendOnly);
    if (!err.ok()) {
      RTC_LOG(LS_ERROR) << "Failed to set video direction with error: " << err.message();
      return;
    }
    chime::LocalVideoConfiguration local_video_configuration;
    // Send it at max 800 kpbs. Modify this value based on number of attendees for better experience.
    local_video_configuration.max_bitrate_kbps = kMaxkbps;
    signaling_client_->UpdateLocalVideo(local_video_transceiver_->mid().value(), local_video_configuration);

    peer_connection_->CreateOffer(
        new rtc::RefCountedObject<CreateSessionDescriptionObserver>(session_description_observer_),
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }
}
// Update the signaling client's information from remote transceivers
bool MeetingController::RemoteVideoChanged() {
  bool is_remote_video_changed = false;
  // Traceievers' mids are set after SubAck call.
  // Therefore, we would need to update remote transeiver with signaling SDK after SubAck
  for (const auto& remote_video_transceiver_pair : attendee_id_to_remote_video_transceivers_) {
    if (remote_video_transceiver_pair.second->mid().has_value()) {
      std::string mid = remote_video_transceiver_pair.second->mid().value();
      // New video
      if (video_sources_to_subscribe_.find(mid) == video_sources_to_subscribe_.end()) {
        RTC_LOG(LS_INFO)
            << "Adding to remote mid of " << mid << " with "
            << attendee_id_to_remote_video_sources_[remote_video_transceiver_pair.first].attendee.attendee_id;
        is_remote_video_changed = true;
        video_sources_to_subscribe_[mid] = attendee_id_to_remote_video_sources_[remote_video_transceiver_pair.first];
      }
    }
  }

  return is_remote_video_changed;
}

void MeetingController::SetExternalVideoSource(rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> track,
                                        webrtc::VideoTrackInterface::ContentHint content_hint) {
  if (!peer_connection_) {
    RTC_LOG(LS_ERROR) << "Cannot set video source before peer connection";
    return;
  }
  if (!track) {
    RTC_LOG(LS_ERROR) << "Given null track";
    return;
  }

  RTC_LOG(LS_INFO) << "Setting external video input track, with content hint: "
                   << static_cast<std::underlying_type<webrtc::VideoTrackInterface::ContentHint>::type>(content_hint);
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> capturer = local_video_source_;

  // Create video track
  auto local_track = peer_connection_factory_->CreateVideoTrack(kVideoCameraLabel.c_str(), capturer);
  local_track->set_content_hint(content_hint);

  if (!IsValidTransceiver(local_video_transceiver_)) {
    RTC_LOG(LS_INFO) << "Building local video transceiver";
    webrtc::RtpTransceiverInit transceiver_init;
    transceiver_init.direction = webrtc::RtpTransceiverDirection::kInactive;
    webrtc::RtpEncodingParameters send_encoding;
    transceiver_init.send_encodings.push_back(send_encoding);
    auto transceiver_or_err = peer_connection_->AddTransceiver(local_track, transceiver_init);
    if (!transceiver_or_err.ok()) {
      RTC_LOG(LS_ERROR) << "Failed to add local video transceiver to PeerConnection: "
                        << transceiver_or_err.error().message();
      return;
    }
    local_video_transceiver_ = transceiver_or_err.MoveValue();
  } else {
    RTC_LOG(LS_INFO) << "Setting Track";
    local_video_transceiver_->sender()->SetTrack(local_track);
  }
}

void MeetingController::SendDataMessage(const std::string& msg) {
  DataMessageToSend message{};
  message.topic = kDataMessageTopic;
  message.lifetime_ms = kDataMessageLifetimeMs;
  message.data = msg;
  signaling_client_->SendDataMessage(message);
}
