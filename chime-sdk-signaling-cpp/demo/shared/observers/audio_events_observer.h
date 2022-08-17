//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef AUDIO_EVENTS_OBSERVER_H_
#define AUDIO_EVENTS_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "utils/attendee.h"
#include "utils/volume_update.h"

#include "controllers/meeting_controller.h"

#include <vector>

using namespace chime;

class AudioEventsObserver : public SignalingClientObserver {
 public:
  void OnVolumeUpdates(const std::vector<VolumeUpdate>& updates) override;
  void OnAttendeeAudioMuted(const Attendee& attendee) override;
  void OnAttendeeAudioUnmuted(const Attendee& attendee) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override {}
  void OnSignalingClientStarted(const SignalingClientStartInfo& start_info) override {}
  void OnSignalingClientStopped(const SignalingClientStatus& status) override {}
};

#endif  // AUDIO_EVENTS_OBSERVER_H_
