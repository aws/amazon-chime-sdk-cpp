// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_MEETING_SESSION_CONFIGURATION_H_
#define CHIME_MEETING_SESSION_CONFIGURATION_H_

#include <memory>
#include <string>

#include "meeting_session_credentials.h"
#include "meeting_session_urls.h"

namespace chime {

// TODO: Fix to not use unique pointers
// Contains information necessary to start a meeting.
struct MeetingSessionConfiguration {
  std::string meeting_id;

  std::string external_meeting_id;

  MeetingSessionCredentials credentials;

  MeetingSessionURLs urls;

  MeetingSessionConfiguration CreateContentConfiguration() {
    return MeetingSessionConfiguration {
      meeting_id,
      external_meeting_id,
      MeetingSessionCredentials {
        credentials.attendee_id + "#content",
        credentials.external_user_id,
        credentials.join_token + "#content"
      },
      MeetingSessionURLs {
        urls.audio_host_url,
        urls.signaling_url
      },
    };
  }

  std::string DebugString() const {
    return "{ meeting_id: " + meeting_id + ", external_meeting_id: " + external_meeting_id +
           ", credentials: " + credentials.DebugString() + ", urls: " + urls.DebugString() + " }";
  }
};
}  // namespace chime

#endif  // CHIME_MEETING_SESSION_CONFIGURATION_H_
