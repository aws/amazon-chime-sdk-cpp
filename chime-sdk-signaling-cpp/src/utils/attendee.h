// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_ATTENDEE_H_
#define CHIME_ATTENDEE_H_

#include <string>

namespace chime {

// Uniquely identifies an attendee in a meeting.
struct Attendee {
  // Globally unique identifier for attendee
  std::string attendee_id;

  // Name of attendee
  std::string external_user_id;

  std::string DebugString() const {
    return "{ attendee_id: " + attendee_id + ", external_user_id: " + external_user_id + " }";
  }
};

}  // namespace chime

#endif  // CHIME_ATTENDEE_H_
