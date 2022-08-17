//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef VIDEO_EVENTS_OBSERVER_H_
#define VIDEO_EVENTS_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "audio_video/remote_video_source_info.h"

#include "controllers/meeting_controller.h"

#include <vector>

using namespace chime;

class VideoEventsObserver : public chime::SignalingClientObserver {
 public:
  VideoEventsObserver(MeetingController* controller, SessionDescriptionObserver* session_description_observer);
  void OnRemoteVideoSourcesAvailable(const std::vector<RemoteVideoSourceInfo>& sources) override;
  void OnRemoteVideoSourcesUnavailable(const std::vector<RemoteVideoSourceInfo>& sources) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override {}
  void OnSignalingClientStarted(const SignalingClientStartInfo& start_info) override {}
  void OnSignalingClientStopped(const SignalingClientStatus& status) override {}

 private:
  MeetingController* controller_ = nullptr;
  SessionDescriptionObserver* session_description_observer_ = nullptr;
};

#endif  // VIDEO_EVENTS_OBSERVER_H_
