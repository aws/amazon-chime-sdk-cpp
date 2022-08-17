//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "video_events_observer.h"

#include "audio_video/remote_video_source_info.h"

#include "session_description_observer_adapter.h"

#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/api/rtp_transceiver_direction.h"
#include "webrtc/api/media_types.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/api/peer_connection_interface.h"

#include <vector>

using namespace chime;

VideoEventsObserver::VideoEventsObserver(MeetingController* controller,
                                         SessionDescriptionObserver* session_description_observer)
    : controller_(controller), session_description_observer_(session_description_observer) {}

void VideoEventsObserver::OnRemoteVideoSourcesAvailable(const std::vector<RemoteVideoSourceInfo>& sources) {
  for (const auto& source : sources) {
    if (controller_->attendee_id_to_remote_video_transceivers_.find(source.attendee.attendee_id) ==
        controller_->attendee_id_to_remote_video_transceivers_.end()) {
      // Create remote transeiver based on the attendee id.
      RTC_LOG(LS_INFO) << "Adding remote transceiver for attendee: " << source.attendee.attendee_id;
      webrtc::RtpTransceiverInit transceiver_init;
      transceiver_init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
      auto transceiver_or_err =
          controller_->peer_connection_->AddTransceiver(cricket::MediaType::MEDIA_TYPE_VIDEO, transceiver_init);
      if (!transceiver_or_err.ok()) {
        RTC_LOG(LS_ERROR) << "Failed to add remote video transceiver to PeerConnection: "
                          << transceiver_or_err.error().message();
        return;
      }

      controller_->attendee_id_to_remote_video_transceivers_[source.attendee.attendee_id] = transceiver_or_err.value();
      controller_->attendee_id_to_remote_video_sources_[source.attendee.attendee_id] = source;
    }
  }

  controller_->peer_connection_->CreateOffer(
      new rtc::RefCountedObject<CreateSessionDescriptionObserver>(session_description_observer_),
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

void VideoEventsObserver::OnRemoteVideoSourcesUnavailable(const std::vector<RemoteVideoSourceInfo>& sources) {
  for (const auto& source : sources) {
    auto remote_video_transceiver_itr = controller_->attendee_id_to_remote_video_transceivers_.find(source.attendee.attendee_id);
    if (remote_video_transceiver_itr != controller_->attendee_id_to_remote_video_transceivers_.end()) {
      // Remove remote transeiver based on the attendee id.
      RTC_LOG(LS_INFO) << "Removing remote transceiver for attendee: " << source.attendee.attendee_id;
      if (remote_video_transceiver_itr->second->mid() && remote_video_transceiver_itr->second->mid().has_value()) {
        controller_->mid_to_remote_video_sinks_.erase(remote_video_transceiver_itr->second->mid().value());
      }
      controller_->attendee_id_to_remote_video_sources_.erase(source.attendee.attendee_id);
      controller_->attendee_id_to_remote_video_transceivers_.erase(source.attendee.attendee_id);
    }
  }

  controller_->peer_connection_->CreateOffer(
      new rtc::RefCountedObject<CreateSessionDescriptionObserver>(session_description_observer_),
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}
