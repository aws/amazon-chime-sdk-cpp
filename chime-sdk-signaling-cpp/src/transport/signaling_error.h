// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */
#ifndef CHIME_SIGNALING_ERROR_H_
#define CHIME_SIGNALING_ERROR_H_

#include <string>

namespace chime {

enum class SignalingErrorType {
  kUnknownFatalError,
  // Fatal needs to restart signaling transport
  kClientFatalError,
  // Non fatal
  kCallAtCapacityError,
};

struct SignalingError {
  SignalingErrorType type = SignalingErrorType::kUnknownFatalError;
  std::string description;
};

}  // namespace chime

#endif  // CHIME_SIGNALING_ERROR_H_
