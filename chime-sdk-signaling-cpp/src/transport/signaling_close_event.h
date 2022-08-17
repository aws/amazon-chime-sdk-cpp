// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_CLOSE_EVENT_H_
#define CHIME_SIGNALING_CLOSE_EVENT_H_

#include <string>

namespace chime {

enum class SignalingCloseCode {
  kUnknown,
  kOK,
  kDisconnected
};

struct SignalingCloseEvent {
  SignalingCloseCode code;
  std::string description;
};

}  // namespace chime

#endif  // CHIME_SIGNALING_CLOSE_EVENT_H_
