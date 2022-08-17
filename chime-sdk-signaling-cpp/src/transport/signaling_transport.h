// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_TRANSPORT_H_
#define CHIME_SIGNALING_TRANSPORT_H_

#include "signaling_transport_observer.h"
#include "utils/runnable.h"
#include "proto/video_control.pb.h"

namespace chime {

class SignalingTransport : public Runnable {
 public:
  virtual ~SignalingTransport() = default;
  // Checks whether signaling client is connected.
  virtual void Start() = 0;
  virtual void Stop() = 0;

  // Send Signal Frame. Internally, it should convert to binary
  virtual bool SendSignalFrame(signal_rtc::SignalFrame& frame) = 0;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_TRANSPORT_H_
