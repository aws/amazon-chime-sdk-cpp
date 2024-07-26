// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_TURN_CREDENTIAL_H_
#define CHIME_SIGNALING_TURN_CREDENTIAL_H_

#include <vector>
#include <cstdint>

namespace chime {

struct TurnCredentials {
  std::string username;
  std::string password;
  uint32_t ttl;
  std::vector<std::string> uris;
};

}  // namespace chime

#endif  // CHIME_SIGNALING_TURN_CREDENTIAL_H_
