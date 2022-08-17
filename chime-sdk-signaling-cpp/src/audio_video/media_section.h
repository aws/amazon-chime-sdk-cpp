// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_MEDIA_SECTION_H_
#define CHIME_SIGNALING_MEDIA_SECTION_H_

namespace chime {
enum class MediaType {
  kAudio,
  kVideo
};

enum class MediaDirection {
  kInactive,
  kSendOnly,
  kRecvOnly,
  kSendRecv,
};

struct MediaSection {
  MediaType type;
  std::string mid;
  MediaDirection direction;
};

} // namespace chime
#endif  // CHIME_SIGNALING_MEDIA_SECTION_H_
