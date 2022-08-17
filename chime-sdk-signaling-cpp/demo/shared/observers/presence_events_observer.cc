//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "presence_events_observer.h"

#include "utils/attendee.h"
#include "utils/signal_strength_update.h"

#include "webrtc/rtc_base/logging.h"

#include <vector>

using namespace chime;

void PresenceEventsObserver::OnAttendeeJoined(const Attendee& attendee) {
  RTC_LOG(LS_INFO) << "Attendee  " << attendee.DebugString() << " has joined the meeting.";
}

void PresenceEventsObserver::OnAttendeeLeft(const Attendee& attendee) {
  RTC_LOG(LS_INFO) << "Attendee  " << attendee.DebugString() << " has left the meeting.";
}

void PresenceEventsObserver::OnAttendeeDropped(const Attendee& attendee) {
  RTC_LOG(LS_INFO) << "Attendee  " << attendee.DebugString() << " has dropped from the meeting.";
}

void PresenceEventsObserver::OnSignalStrengthChanges(const std::vector<SignalStrengthUpdate>& updates) {
  for (const auto& update : updates) {
    RTC_LOG(LS_VERBOSE) << "Attendee " << update.attendee.DebugString() << " updated to "
                        << update.signal_strength.normalized_signal_strength;
  }
}
