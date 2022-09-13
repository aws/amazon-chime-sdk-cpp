//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "mocks/mock_signaling_transport.h"
#include "mocks/mock_signaling_observer.h"

#include "signaling/default_signaling_client.h"

#include "gtest/gtest.h"
#include "websocket/websocket.h"
#include <memory>

using namespace chime;

class MockSignalingTransportFactory: public SignalingTransportFactory {
 public:
  MockSignalingTransportFactory(std::unique_ptr<SignalingTransport> transport): transport_(std::move(transport)) {};
  std::unique_ptr<SignalingTransport> CreateSignalingTransport(SignalingTransportConfiguration configuration,
                                                               SignalingTransportObserver* observer) override {
    return std::move(transport_);
  }
  std::unique_ptr<SignalingTransport> transport_;
};

class DefaultSignalingClientTest : public testing::Test {
 public:
  std::unique_ptr<MockSignalingTransport> mock_signaling_transport_;
  SignalingTransport* transport_;

  void SetUp() override {
    mock_signaling_transport_ = std::make_unique<MockSignalingTransport>();
    transport_ = mock_signaling_transport_.get();
  }
};

TEST_F(DefaultSignalingClientTest, ShouldReturnNonNull) {
  SignalingClientConfiguration configuration;
  DefaultSignalingDependencies dependencies {};
  dependencies.signal_transport_factory = std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_));
  std::unique_ptr<DefaultSignalingClient> client =
      std::make_unique<DefaultSignalingClient>(std::move(configuration), std::move(dependencies));

  EXPECT_TRUE(client != nullptr);
}

TEST_F(DefaultSignalingClientTest, ShouldCallStart) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
    std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };
  
  EXPECT_CALL(*mock_signaling_transport_ref, Start());

  client.Start();
}

TEST_F(DefaultSignalingClientTest, ShouldCallSendLeave) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };

  // Join + Leave
  EXPECT_CALL(*mock_signaling_transport_ref, SendSignalFrame).Times(2);
  client.OnSignalingConnected();

  client.Stop();
}

TEST_F(DefaultSignalingClientTest, ShouldCallPollIfGivenOptionIsUsePoll) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };

  EXPECT_CALL(*mock_signaling_transport_ref, Poll());

  client.Poll();
}

TEST_F(DefaultSignalingClientTest, ShouldCallSendSignalFrameWhenConnected) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };

  // Make the state to be connected
  // Update + Join + Leave
  EXPECT_CALL(*mock_signaling_transport_ref, SendSignalFrame).Times(3);
  client.OnSignalingConnected();

  client.SendUpdates();
}

TEST_F(DefaultSignalingClientTest, ShouldNotCallSendSignalFrameWhenNotConnected) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };

  EXPECT_CALL(*mock_signaling_transport_ref, SendSignalFrame).Times(0);

  client.SendUpdates();
}

TEST_F(DefaultSignalingClientTest, ShouldCallSendJoinWhenConnected) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };

  // Make the state to be connected
  // Join + Leave when destructed
  EXPECT_CALL(*mock_signaling_transport_ref, SendSignalFrame).Times(2);

  client.OnSignalingConnected();
}

TEST_F(DefaultSignalingClientTest, ShouldCallOnSignalingClientStoppedWhenClosed) {
  SignalingClientConfiguration configuration;
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };
  MockSignalingClientObserver observer;
  client.AddSignalingClientObserver(&observer);
  SignalingCloseEvent event{};
  event.description = "OK";
  event.code = SignalingCloseCode::kOK;

  EXPECT_CALL(observer, OnSignalingClientStopped).Times(1);

  client.OnSignalingClosed(event);
}

TEST_F(DefaultSignalingClientTest, ShouldCallSendDataMessageWhenSendDataMessage) {
  SignalingClientConfiguration configuration;
  auto* mock_signaling_transport_ref = mock_signaling_transport_.get();
  DefaultSignalingDependencies dependencies {
      std::make_unique<MockSignalingTransportFactory>(std::move(mock_signaling_transport_))
  };
  DefaultSignalingClient client {
      configuration,
      std::move(dependencies)
  };
  // Join + data message + leave
  EXPECT_CALL(*mock_signaling_transport_ref, SendSignalFrame).Times(3);
  
  client.OnSignalingConnected();

  DataMessageToSend data_message_to_send;
  data_message_to_send.lifetime_ms = 1000;
  data_message_to_send.data = "Hello world";
  data_message_to_send.topic = "topic";
  client.SendDataMessage(data_message_to_send);
}



