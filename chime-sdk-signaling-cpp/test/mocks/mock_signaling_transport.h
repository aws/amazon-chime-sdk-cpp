//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_H_
#define CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_H_

#include "transport/signaling_transport.h"
#include "gmock/gmock.h"

using namespace chime;

class MockSignalingTransport : public SignalingTransport {
 public:
  MOCK_METHOD(void, Start, (), (override));
  MOCK_METHOD(void, Stop, (), (override));
  MOCK_METHOD(bool, SendSignalFrame, (signal_rtc::SignalFrame & frame), (override));
  MOCK_METHOD(void, Poll, (), (override));
};

#endif  // CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_H_
