//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef PRESENCE_EVENTS_OBSERVER_H_
#define PRESENCE_EVENTS_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "utils/attendee.h"
#include "utils/signal_strength_update.h"

#include "controllers/meeting_controller.h"

#include <vector>

using namespace chime;

class PresenceEventsObserver : public chime::SignalingClientObserver {
 public:
  void OnAttendeeJoined(const Attendee& attendee) override;
  void OnAttendeeLeft(const Attendee& attendee) override;
  void OnAttendeeDropped(const Attendee& attendee) override;
  void OnSignalStrengthChanges(const std::vector<SignalStrengthUpdate>& updates) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override {}
  void OnSignalingClientStarted(const SignalingClientStartInfo& start_info) override {}
  void OnSignalingClientStopped(const SignalingClientStatus& status) override {}
};

#endif  // PRESENCE_EVENTS_OBSERVER_H_
