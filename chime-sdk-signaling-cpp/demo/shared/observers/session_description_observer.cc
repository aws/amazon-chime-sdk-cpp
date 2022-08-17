//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "session_description_observer.h"

#include "controllers/meeting_controller.h"

#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/jsep.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/api/rtc_error.h"
#include "webrtc/api/peer_connection_interface.h"

#include <vector>

using namespace chime;

void SessionDescriptionObserver::OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) {
  RTC_LOG(LS_INFO) << "Session Description created successfully.";
  controller_->peer_connection_->SetLocalDescription(new rtc::RefCountedObject<SetSessionDescriptionObserver>(true, this), desc);
}

void SessionDescriptionObserver::OnCreateSessionDescriptionFailure(const webrtc::RTCError& error) {
  RTC_LOG(LS_ERROR) << "Session Description failed to create.";
}

void SessionDescriptionObserver::OnSetSessionDescriptionSuccess(bool is_local) {
  if (is_local) {
    RTC_LOG(LS_INFO) << "Local session description set successfully. "
                        "Attempting to send updates.";
    controller_->SendUpdates();
  } else {
    RTC_LOG(LS_INFO) << "Remote session description set successfully.";
    controller_->updates_in_flight_ = false;

    // If there is additional mids that are set from
    // remote transeivers, update the signaling client about the info
    // and resubscribe.
    if (controller_->RemoteVideoChanged()) {
      std::vector<std::string> removed;
      controller_->signaling_client_->UpdateRemoteVideoSubscriptions(controller_->video_sources_to_subscribe_, removed);
      controller_->peer_connection_->CreateOffer(new rtc::RefCountedObject<CreateSessionDescriptionObserver>(this),
                                    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
  }
}

void SessionDescriptionObserver::OnSetSessionDescriptionFailure(bool is_local, const webrtc::RTCError& error) {
  if (is_local) {
    RTC_LOG(LS_ERROR) << "Local description failed to set. Error: " << error.message();
  } else {
    RTC_LOG(LS_ERROR) << "Remote description failed to set. Error: " << error.message();
    controller_->updates_in_flight_ = false;
  }
}
