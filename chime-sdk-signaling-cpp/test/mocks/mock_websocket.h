//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_MOCK_WEBSOCKET_H_
#define CHIME_SIGNALING_MOCK_WEBSOCKET_H_

#include "websocket/websocket.h"

#include "gmock/gmock.h"

#include <vector>

using namespace chime;

class MockWebsocket: public Websocket {
 public:
  MOCK_METHOD(void, Poll, (), (override));
  MOCK_METHOD(void, StopRun, (), (override));
  MOCK_METHOD(void, Run, (), (override));
  MOCK_METHOD(void, Connect, (), (override));
  MOCK_METHOD(void, Close, (), (override));
  MOCK_METHOD(void, SendBinary, (const std::vector<uint8_t>& data), (override));
  ~MockWebsocket() override = default;
  bool IsPollable() override { return true; };
};

#endif  // CHIME_SIGNALING_MOCK_WEBSOCKET_H_
