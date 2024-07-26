//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_OBSERVER_H_
#define CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_OBSERVER_H_

#include "gmock/gmock.h"

#include "transport/signaling_transport_observer.h"

using namespace chime;

class MockSignalingTransportObserver : public SignalingTransportObserver {
 public:
  MOCK_METHOD(void, OnSignalFrameReceived, (const signal_sdk::SdkSignalFrame& frame), (override));
  MOCK_METHOD(void, OnSignalingConnected, (), (override));
  MOCK_METHOD(void, OnSignalingErrorReceived, (const SignalingError& error), (override));
  MOCK_METHOD(void, OnSignalingClosed, (const SignalingCloseEvent& event), (override));
};


#endif  // CHIME_SIGNALING_MOCK_SIGNALING_TRANSPORT_OBSERVER_H_
