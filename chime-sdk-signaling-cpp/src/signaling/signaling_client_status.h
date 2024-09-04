// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_STATUS_H_
#define CHIME_SIGNALING_STATUS_H_

namespace chime {

enum class SignalingClientStatusType { kNetworkError, kOk, kUnknown, kClientError, kJoinFromAnotherDevice, kMeetingEnded, kBadRequest, kServerInternalError };
struct SignalingClientStatus {
  SignalingClientStatusType type = SignalingClientStatusType::kUnknown;
  std::string reason = "";
};

static bool IsSignalingClientStatusTypeFatal(SignalingClientStatusType type) {
    return type == SignalingClientStatusType::kJoinFromAnotherDevice || type == SignalingClientStatusType::kMeetingEnded || type == SignalingClientStatusType::kUnknown || 
            type == SignalingClientStatusType::kBadRequest || type == SignalingClientStatusType::kClientError;
}

}  // namespace chime
#endif  // CHIME_SIGNALING_STATUS_H_
