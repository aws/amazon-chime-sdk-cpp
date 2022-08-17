// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_MEETING_SESSION_CREDENTIALS_H_
#define CHIME_MEETING_SESSION_CREDENTIALS_H_

#include <string>

namespace chime {

// Contains credentials necessary to authenticate an attendee for a meeting.
struct MeetingSessionCredentials {
  std::string attendee_id;

  std::string external_user_id;

  std::string join_token;

  std::string DebugString() const {
    return "{ attendee_id: " + attendee_id + ", external_user_id: " + external_user_id +
           ", join_token: " + join_token + " }";
  };
};
}  // namespace chime

#endif  // CHIME_MEETING_SESSION_CREDENTIALS_H_
