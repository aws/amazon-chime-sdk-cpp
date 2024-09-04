//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "mocks/mock_websocket.h"
#include "mocks/mock_signaling_transport_observer.h"

#include "transport/websocket_signaling_transport.h"

#include "gtest/gtest.h"

using namespace chime;

class MockWebsocketFactory : public WebsocketFactory {
 public:
  MockWebsocketFactory(std::unique_ptr<Websocket> websocket) : websocket_(std::move(websocket)) {}
  std::unique_ptr<Websocket> CreateWebsocket(WebsocketConfiguration configuration,
                                             WebsocketObserver* observer) override {
    return std::move(websocket_);
  }

  std::unique_ptr<Websocket> websocket_;
};

class WebsocketSignalingTransportTest : public testing::Test {
 public:
  std::unique_ptr<MockWebsocket> mock_websocket_;
  SignalingTransportConfiguration configuration_;

  void SetUp() override {
    configuration_ = {{"audio_host_url", "signaling_url"}, {"attendee_id", "external_user_id", "join_token"}};
    mock_websocket_ = std::make_unique<MockWebsocket>();
  }
};

TEST_F(WebsocketSignalingTransportTest, ShouldReturnNonNull) {
  SignalingTransportConfiguration configuration;
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));
  std::unique_ptr<WebsocketSignalingTransport> transport =
      std::make_unique<WebsocketSignalingTransport>(configuration_, &mock_observer, std::move(factory));

  EXPECT_TRUE(transport != nullptr);
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallConnectWhenStart) {
  auto* mock_websocket_ref = mock_websocket_.get();
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{configuration_, &mock_observer, std::move(factory)};

  EXPECT_CALL(*mock_websocket_ref, Connect()).Times(1);

  transport.Start();
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallCloseWhenStop) {
  MockSignalingTransportObserver mock_observer;
  auto* mock_websocket_ref = mock_websocket_.get();
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{configuration_, &mock_observer, std::move(factory)};

  // +1 for destructor
  EXPECT_CALL(*mock_websocket_ref, Close()).Times(2);
  transport.Start();
  transport.Stop();
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallPollWhenPoll) {
  SignalingTransportConfiguration configuration;

  auto* mock_websocket_ref = mock_websocket_.get();
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  EXPECT_CALL(*mock_websocket_ref, Poll()).Times(1);

  transport.Poll();
}

TEST_F(WebsocketSignalingTransportTest, ShouldSendBinaryPollWhenSendSignalFrame) {
  SignalingTransportConfiguration configuration;
  auto* mock_websocket_ref = mock_websocket_.get();
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  signal_sdk::SdkSignalFrame frame;
  frame.set_type(signal_sdk::SdkSignalFrame_Type_JOIN);
  signal_sdk::SdkJoinFrame* join_frame = frame.mutable_join();
  join_frame->set_max_num_of_videos(25);
  join_frame->set_protocol_version(2);
  uint32_t flags = signal_sdk::HAS_STREAM_UPDATE;
  join_frame->set_flags(flags);

  // Unfortunately, hard to get input for the vector due to set_timestamp_ms
  EXPECT_CALL(*mock_websocket_ref, SendBinary(testing::_)).Times(1);

  transport.SendSignalFrame(frame);
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallOnConnectedWhenOnConnected) {
  SignalingTransportConfiguration configuration;
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  EXPECT_CALL(mock_observer, OnSignalingConnected()).Times(1);

  transport.OnWebsocketConnected();
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallOnSignalingErrorReceivedWhenOnWebsocketError) {
  SignalingTransportConfiguration configuration;
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  EXPECT_CALL(mock_observer, OnSignalingErrorReceived(testing::_)).Times(1);

  WebsocketErrorStatus error_status;
  error_status.description = "error";
  transport.OnWebsocketError(error_status);
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallOnSignalingClosedWhenOnWebsocketClosed) {
  SignalingTransportConfiguration configuration;
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  EXPECT_CALL(mock_observer, OnSignalingClosed(testing::_)).Times(1);

  WebsocketStatus status;
  status.description = "OK";
  transport.OnWebsocketClosed(status);
}

TEST_F(WebsocketSignalingTransportTest, ShouldCallOnSignalFrameReceivedWhenOnWebsocketBinaryReceived) {
  SignalingTransportConfiguration configuration;
  MockSignalingTransportObserver mock_observer;
  std::unique_ptr<WebsocketFactory> factory = std::make_unique<MockWebsocketFactory>(std::move(mock_websocket_));

  WebsocketSignalingTransport transport{std::move(configuration), &mock_observer, std::move(factory)};

  std::string buf;
  signal_sdk::SdkSignalFrame frame;
  frame.set_type(signal_sdk::SdkSignalFrame_Type::SdkSignalFrame_Type_AUDIO_STREAM_ID_INFO);
  signal_sdk::SdkAudioStreamIdInfoFrame* stream_frame = frame.mutable_audio_stream_id_info();
  auto* stream = stream_frame->add_streams();
  stream->set_audio_stream_id(1);
  stream->set_attendee_id("attendee-1");
  stream->set_external_user_id("external-user-1");
  auto current_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
  frame.set_timestamp_ms(current_ms.count());

  frame.SerializeToString(&buf);

  // TODO: need to implement comparator for frame
  EXPECT_CALL(mock_observer, OnSignalFrameReceived(testing::_)).Times(1);

  std::vector<uint8_t> input(buf.size() + 1);
  input[0] = 2;
  std::copy(buf.begin(), buf.end(), input.begin() + 1);
  transport.OnWebsocketBinaryReceived(input);
}
