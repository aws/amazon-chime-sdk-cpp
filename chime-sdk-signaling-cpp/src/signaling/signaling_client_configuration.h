// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_CLIENT_CONFIGURATION_H_
#define CHIME_SIGNALING_CLIENT_CONFIGURATION_H_

#include "session/meeting_session_configuration.h"

namespace chime {
/**
 * Defines configuration needed to create signaling client
 */
struct SignalingClientConfiguration {
  MeetingSessionConfiguration meeting_configuration;

  /* @unstable */
  // Enable attendee update.
  // Disabling assumes that attendee data will come from different source.
  bool enable_attendee_update = true;
  // Whether the audio is muted initially 
  bool mute_on_join = false;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_CLIENT_CONFIGURATION_H_
