//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef TEST_LIFECYCLE_OBSERVER_H_
#define TEST_LIFECYCLE_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "signaling/signaling_client_start_info.h"
#include "signaling/signaling_client_status.h"

#include "controllers/meeting_controller.h"
#include "observers/lifecycle_observer.h"
#include "observers/peer_connection_observer.h"
#include "observers/video_events_observer.h"
#include "observers/session_description_observer.h"
#include "utils/test_marker.h"

#include <string>

using namespace chime;

class TestLifecycleObserver : public LifecycleObserver {
 public:
  TestLifecycleObserver(MeetingController* controller, std::shared_ptr<TestMarker> marker,
                        PeerConnectionObserver* peer_connection_observer, VideoEventsObserver* video_events_observer,
                        SessionDescriptionObserver* session_description_observer);
  void OnSignalingClientStarted(const SignalingClientStartInfo& join_info) override;
  void OnSignalingClientStopped(const SignalingClientStatus& status) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override;

 private:
  std::shared_ptr<TestMarker> marker_;
};

#endif  // TEST_LIFECYCLE_OBSERVER_H_
