//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef LIFECYCLE_OBSERVER_H_
#define LIFECYCLE_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "signaling/signaling_client_start_info.h"
#include "signaling/signaling_client_status.h"

#include "controllers/meeting_controller.h"
#include "observers/peer_connection_observer.h"
#include "observers/video_events_observer.h"
#include "observers/session_description_observer.h"

#include <string>

using namespace chime;

class LifecycleObserver : public SignalingClientObserver {
 public:
  LifecycleObserver(MeetingController* controller,
                    PeerConnectionObserver* peer_connection_observer,
                    VideoEventsObserver* video_events_observer,
                    SessionDescriptionObserver* session_description_observer);
  void OnSignalingClientStarted(const SignalingClientStartInfo& join_info) override;
  void OnSignalingClientStopped(const SignalingClientStatus& status) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override;

 protected:
  MeetingController* controller_ = nullptr;
  PeerConnectionObserver* peer_connection_observer_ = nullptr;
  VideoEventsObserver* video_events_observer_ = nullptr;
  SessionDescriptionObserver* session_description_observer_ = nullptr;
};

#endif  // LIFECYCLE_OBSERVER_H_
