//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "lifecycle_observer.h"

#include "signaling/signaling_client_start_info.h"
#include "signaling/signaling_client_status.h"

#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/pc/session_description.h"
#include "webrtc/api/audio_options.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/pc/video_track_source.h"
#include "webrtc/api/media_stream_interface.h"
#include "webrtc/api/jsep.h"

#include <string>
#include <memory>

using namespace chime;

LifecycleObserver::LifecycleObserver(MeetingController* controller,
                                     PeerConnectionObserver* peer_connection_observer,
                                     VideoEventsObserver* video_events_observer,
                                     SessionDescriptionObserver* session_description_observer)
    : controller_(controller),
      peer_connection_observer_(peer_connection_observer),
      video_events_observer_(video_events_observer),
      session_description_observer_(session_description_observer)
{}

void LifecycleObserver::OnSignalingClientStarted(const SignalingClientStartInfo& join_info) {
  RTC_LOG(LS_INFO) << "Signaling Client started.";

  RTC_LOG(LS_INFO) << "Setting TURN credentials.";
  auto creds = join_info.credentials;
  webrtc::PeerConnectionInterface::IceServer server;
  server.urls = creds.uris;
  server.username = creds.username;
  server.password = creds.password;

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  config.servers.push_back(server);

  webrtc::PeerConnectionDependencies dependencies(peer_connection_observer_);
  auto result = controller_->peer_connection_factory_->CreatePeerConnectionOrError(config, std::move(dependencies));
  if (!result.ok()) {
    RTC_LOG(LS_ERROR) << "Failed to create peer connection.";
    return;
  } else {
    controller_->peer_connection_ = result.MoveValue();
  }

  const char kAudioLabel[] = "external_audio";
  auto audio_source = controller_->peer_connection_factory_->CreateAudioSource(cricket::AudioOptions());
  auto audio_track = controller_->peer_connection_factory_->CreateAudioTrack(kAudioLabel, audio_source.get());

  webrtc::RtpTransceiverInit transceiver_init;
  transceiver_init.direction = webrtc::RtpTransceiverDirection::kSendRecv;
  auto transceiver_or_error = controller_->peer_connection_->AddTransceiver(audio_track, transceiver_init);
  if (!transceiver_or_error.ok()) {
    RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection; err:" << transceiver_or_error.error().message();
    return;
  }
  transceiver_or_error.value()->sender()->SetTrack(audio_track.get());

  controller_->local_video_source_ = new rtc::RefCountedObject<FakeVideoSource>();

  controller_->SetExternalVideoSource(controller_->local_video_source_, webrtc::VideoTrackInterface::ContentHint::kNone);

  // Handle if there is any video sources from beginning
  if (!join_info.sources.empty()) {
    video_events_observer_->OnRemoteVideoSourcesAvailable(join_info.sources);
  }

  RTC_LOG(LS_INFO) << "Creating SDP offer.";
  controller_->peer_connection_->CreateOffer(
      new rtc::RefCountedObject<CreateSessionDescriptionObserver>(session_description_observer_),
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

void LifecycleObserver::OnSignalingClientStopped(const SignalingClientStatus& status) {
  RTC_LOG(LS_INFO) << "Signaling client has stopped.";
  controller_->peer_connection_ = nullptr;
}

void LifecycleObserver::OnRemoteDescriptionReceived(const std::string& sdp_answer) {
  RTC_LOG(LS_VERBOSE) << "OnRemoteDescriptionReceived: " + sdp_answer;
  webrtc::SdpParseError error;
  std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
      webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, sdp_answer, &error);
  if (!session_description) {
    RTC_LOG(LS_ERROR) << "Failed to parse sdp answer. Error: " << error.description;
    return;
  }

  controller_->peer_connection_->SetRemoteDescription(
      new rtc::RefCountedObject<SetSessionDescriptionObserver>(false, session_description_observer_),
      session_description.release());
}
