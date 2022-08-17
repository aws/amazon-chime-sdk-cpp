// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_STATUS_H_
#define CHIME_SIGNALING_STATUS_H_

namespace chime {

enum class SignalingClientStatusType { kNetworkError, kOk, kUnknown, kClientError };

struct SignalingClientStatus {
  SignalingClientStatusType type = SignalingClientStatusType::kUnknown;
  std::string reason = "";
};

}  // namespace chime
#endif  // CHIME_SIGNALING_STATUS_H_
