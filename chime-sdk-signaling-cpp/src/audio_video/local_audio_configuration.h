// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_LOCAL_AUDIO_CONFIGURATION_H_
#define CHIME_SIGNALING_LOCAL_AUDIO_CONFIGURATION_H_

#include "audio_video/mute_state.h"

namespace chime {
/**
 * Defines local audio configuration, which builders will
 * assign it using `SignalingClient.AddLocalAudio` or `SignalingClient.UpdateLocalAudio`
 * It defaults to unmuted status
 */
struct LocalAudioConfiguration {
  MuteState mute_state = MuteState::kUnmute;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_LOCAL_AUDIO_CONFIGURATION_H_
