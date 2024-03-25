// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */
#include "websocket_signaling_transport.h"

#include "utils/logging.h"

#include <chrono>
#include <utility>

namespace chime {

namespace {
constexpr int video_version = 3;
constexpr int video_type = 2;
}  // namespace

void WebsocketSignalingTransport::Start() {
  if (!is_stopped_) return;

  CHIME_LOG(LogLevel::kInfo, "Starting Signaling Transport")

  websocket_->Connect();
  is_stopped_ = false;
}

void WebsocketSignalingTransport::Stop() {
  if (is_stopped_) return;
  CHIME_LOG(LogLevel::kInfo, "Stopping Signaling Transport")

  websocket_->Close();
}

bool WebsocketSignalingTransport::SendSignalFrame(signal_rtc::SignalFrame& frame) {
  const auto current_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
  frame.set_timestamp_ms(current_ms.count());

  LogLevel signaling_log_level =
      frame.type() != signal_rtc::SignalFrame::CLIENT_METRIC ? LogLevel::kInfo : LogLevel::kVerbose;
  CHIME_LOG(signaling_log_level, "Sending control message: type=" + signal_frame_type_strings[frame.type()]);

  std::string buff;
  if (!frame.SerializeToString(&buff)) {
    // If we failed to send a message we should stop since our state
    // might not be recoverable unless we restart everything
    CHIME_LOG(LogLevel::kError, "Unable to serialize string error")

    return false;
  }

  std::vector<uint8_t> data(buff.length() + 1);
  // Add type for the first byte.
  data[0] = video_type;
  std::copy(buff.begin(), buff.end(), data.begin() + 1);

  websocket_->SendBinary(data);

  return true;
}

void WebsocketSignalingTransport::Poll() { websocket_->Poll(); }

WebsocketSignalingTransport::WebsocketSignalingTransport(SignalingTransportConfiguration configuration,
                                                         SignalingTransportObserver* transport_observer,
                                                         std::unique_ptr<WebsocketFactory> websocket_factory) {
  signaling_url_ = configuration.urls.signaling_url;
  join_token_ = configuration.credentials.join_token;
  CHIME_LOG(LogLevel::kInfo, "Creating WebsocketSignalingTransport")
  WebsocketConfiguration websocket_configuration;
  websocket_configuration.url = signaling_url_;
  websocket_configuration.additional_headers["Cookie:"] = std::string("_aws_wt_session=") + join_token_;
  websocket_configuration.additional_headers["X-Chime-Control-Protocol-Version:"] = std::to_string(video_version);
  websocket_configuration.level = LogLevel::kError;
  std::unique_ptr<Websocket> websocket = websocket_factory->CreateWebsocket(websocket_configuration, this);
  websocket_ = std::move(websocket);
  observer_ = transport_observer;
}

WebsocketSignalingTransport::~WebsocketSignalingTransport() {
  if (is_stopped_) return;

  websocket_->StopRun();
  websocket_->Close();
}

void WebsocketSignalingTransport::OnWebsocketConnected() { observer_->OnSignalingConnected(); }

void WebsocketSignalingTransport::OnWebsocketClosed(WebsocketStatus status) {
  is_stopped_ = true;
  SignalingCloseEvent event {};
  event.description = status.description;
  event.code = SignalingCloseCode::kOK;
  observer_->OnSignalingClosed(event);
}

void WebsocketSignalingTransport::OnWebsocketError(WebsocketErrorStatus error) {
  is_stopped_ = true;
  SignalingError signaling_error{SignalingErrorType::kClientFatalError, error.description};
  observer_->OnSignalingErrorReceived(signaling_error);
}

void WebsocketSignalingTransport::OnWebsocketBinaryReceived(const std::vector<uint8_t>& data) {
  if (data.size() < 2) return;
  signal_rtc::SignalFrame signal_frame;
  // First byte is for type. This is currently always value 2.
  if (!signal_frame.ParseFromArray(&data[1], static_cast<int>(data.size() - 1))) {
    CHIME_LOG(LogLevel::kError, "Unable to cast to signaling frame")
    return;
  }

  LogLevel log_level = LogLevel::kInfo;

  if (signal_frame.type() == signal_rtc::SignalFrame::BITRATES ||
      signal_frame.type() == signal_rtc::SignalFrame::AUDIO_METADATA) {
    log_level = LogLevel::kVerbose;
  }

  CHIME_LOG(log_level, "Received control message: type=" + signal_frame_type_strings[signal_frame.type()])

  observer_->OnSignalFrameReceived(signal_frame);
}

void WebsocketSignalingTransport::Run() { websocket_->Run(); }

void WebsocketSignalingTransport::StopRun() { websocket_->StopRun(); }

bool WebsocketSignalingTransport::IsPollable() { return websocket_->IsPollable(); }

}  // namespace chime
