// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_LOCAL_VIDEO_CONFIGURATION_H_
#define CHIME_SIGNALING_LOCAL_VIDEO_CONFIGURATION_H_

#include <stdint.h>

namespace chime {
/**
 * Defines local video configuration, which builders will
 * assign to send their local video using `SignalingClient.AddLocalVideo` or `SignalingClient.UpdateLocalVideo`.
 */
struct LocalVideoConfiguration {
  // maximum bit rates of local video
  uint32_t max_bitrate_kbps;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_LOCAL_VIDEO_CONFIGURATION_H_
