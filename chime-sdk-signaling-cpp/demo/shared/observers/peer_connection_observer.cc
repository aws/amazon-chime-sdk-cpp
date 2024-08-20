//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "peer_connection_observer.h"

#include "video/write_to_file_yuv_video_sink.h"

#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/api/jsep.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/api/media_stream_interface.h"

#include <string>
#include <algorithm>
#include <memory>

using namespace chime;

PeerConnectionObserver::PeerConnectionObserver(MeetingController* controller) : controller_(controller) {}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  if (!candidate) return;

  std::string url = candidate->server_url();
  RTC_LOG(LS_INFO) << "OnIceCandidate. Url: " << url;
  // We immediately sending updates when new TURN candidates are received
  if (url.find("turn") == std::string::npos) return;
  controller_->SendUpdates();
}

void PeerConnectionObserver::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  for (const auto& attendee_id_to_remote_transceiver : controller_->attendee_id_to_remote_video_transceivers_) {
    if (attendee_id_to_remote_transceiver.second->receiver()->id() == receiver->id()) {
      RTC_LOG(LS_INFO) << "Removing remote transceiver for attendee ID " << attendee_id_to_remote_transceiver.first;

      controller_->mid_to_remote_video_sinks_.erase(attendee_id_to_remote_transceiver.second->mid().value());
      attendee_id_to_remote_transceiver.second->StopStandard();
      controller_->attendee_id_to_remote_video_transceivers_.erase(
          attendee_id_to_remote_transceiver.first);  // remove from our local variables
      controller_->attendee_id_to_remote_video_sources_.erase(attendee_id_to_remote_transceiver.second->mid().value());
      break;
    }
  }
}

void PeerConnectionObserver::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = transceiver->receiver()->track();

  if (track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind) {
    // Not doing anything for audio track
    return;
  }

  // Find this transceiver in our own map
  auto attendee_id_to_remote_video_transceivers_itr =
      std::find_if(controller_->attendee_id_to_remote_video_transceivers_.begin(),
                   controller_->attendee_id_to_remote_video_transceivers_.end(),
                   [&](const auto& attendee_id_to_remote_transceiver) {
                     return attendee_id_to_remote_transceiver.second->mid() == transceiver->mid();
                   });

  if (attendee_id_to_remote_video_transceivers_itr == controller_->attendee_id_to_remote_video_transceivers_.end()) {
    RTC_LOG(LS_ERROR) << "No remote transceiver found in OnTrack for transceiver with mid: "
                      << transceiver->mid().value_or("unknown");
    return;
  }

  webrtc::VideoTrackInterface* video_track = static_cast<webrtc::VideoTrackInterface*>(track.get());
  auto video_sink = std::make_unique<WriteToFileYuvVideoSink>(transceiver->mid().value());
  rtc::VideoSinkWants wants{};
  wants.rotation_applied = true;
  video_track->AddOrUpdateSink(video_sink.get(), wants);
  // TODO: change it
  controller_->mid_to_remote_video_sinks_[transceiver->mid().value()] = std::move(video_sink);
  RTC_LOG(LS_INFO) << "OnTrack called for video track with track label " << video_track->id();
}
