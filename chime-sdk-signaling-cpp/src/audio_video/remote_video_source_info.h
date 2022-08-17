// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_REMOTE_VIDEO_SOURCE_INFO_H_
#define CHIME_SIGNALING_REMOTE_VIDEO_SOURCE_INFO_H_

#include "utils/attendee.h"

namespace chime {
/**
 * Defines remote video configuration, which builders can modify
 * some properties and resubscribe by `UpdateRemoteVideoSubscriptions`
 */
struct RemoteVideoSourceInfo {
  Attendee attendee;
  /* @unstable */
  uint32_t max_bitrate_kbps{};
  uint32_t stream_id{};
};

} // namespace chime
#endif  // CHIME_SIGNALING_REMOTE_VIDEO_SOURCE_INFO_H_
