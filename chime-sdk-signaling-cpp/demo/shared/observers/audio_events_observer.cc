//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "audio_events_observer.h"

#include "utils/volume_update.h"
#include "utils/attendee.h"

#include "webrtc/rtc_base/logging.h"

#include <vector>

using namespace chime;

void AudioEventsObserver::OnVolumeUpdates(const std::vector<VolumeUpdate>& updates) {
  for (const auto& update : updates) {
    RTC_LOG(LS_VERBOSE) << "Attendee " << update.attendee.DebugString() << " updated to "
                        << update.volume.normalized_volume;
  }
}

void AudioEventsObserver::OnAttendeeAudioMuted(const Attendee& attendee) {
  RTC_LOG(LS_INFO) << "Attendee  " << attendee.DebugString() << " has muted.";
}

void AudioEventsObserver::OnAttendeeAudioUnmuted(const Attendee& attendee) {
  RTC_LOG(LS_INFO) << "Attendee  " << attendee.DebugString() << " has unmuted.";
}
