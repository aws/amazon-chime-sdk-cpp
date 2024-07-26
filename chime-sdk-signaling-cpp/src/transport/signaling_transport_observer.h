// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#ifndef CHIME_SIGNALING_TRANSPORT_OBSERVER_H_
#define CHIME_SIGNALING_TRANSPORT_OBSERVER_H_

#include "signaling_error.h"
#include "signaling_close_event.h"
#include "video_control_sdk.pb.h"


namespace chime {

class SignalingTransportObserver {
 public:
  // signal_sdk comes from protobuf definition
  virtual void OnSignalFrameReceived(const signal_sdk::SdkSignalFrame& frame) = 0;
  virtual void OnSignalingConnected() = 0;
  virtual void OnSignalingErrorReceived(const SignalingError& error) = 0;
  virtual void OnSignalingClosed(const SignalingCloseEvent& event) = 0;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_TRANSPORT_OBSERVER_H_
