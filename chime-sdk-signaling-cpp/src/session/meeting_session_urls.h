// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_MEETING_SESSION_URLS_H_
#define CHIME_MEETING_SESSION_URLS_H_

#include <string>

namespace chime {

// Contains the URLs that will be used to reach the meeting service.
struct MeetingSessionURLs {
  std::string audio_host_url;

  std::string signaling_url;

  std::string DebugString() const {
    return "{ audioHostUrl: " + audio_host_url + ", signalingUrl: " + signaling_url + " }";
  };
};
}  // namespace chime

#endif  // CHIME_MEETING_SESSION_URLS_H_
