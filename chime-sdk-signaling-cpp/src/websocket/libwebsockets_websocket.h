// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SIGNALING_SDK_LIBWEBSOCKETS_WEBSOCKET_H
#define SIGNALING_SDK_LIBWEBSOCKETS_WEBSOCKET_H

#include "websocket.h"
#include "utils/logger/log_level.h"

#include "libwebsockets.h"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <queue>

namespace chime {

struct LibwebsocketsWebsocketConfiguration : public WebsocketConfiguration {
  uint16_t ping_pong_interval_sec = 10;  // Seconds without proof of a valid connection before sending a ping.
  uint16_t idle_timeout_sec = 300;       // Seconds without proof of a valid connection before hanging up.
  int backoff_ms[5] = {1000, 2000, 3000, 4000, 5000};  // Backoff times used in order of index.
  uint8_t retry_percent_jitter_added = 20;                 // Artificial random jitter added to retry attempts.
};

class LibwebsocketsWebsocket : public Websocket {
 public:
  LibwebsocketsWebsocket(LibwebsocketsWebsocketConfiguration configuration, WebsocketObserver* observer);
  ~LibwebsocketsWebsocket() = default;
  // chime::signaling::Websocket overrides
  void Connect() override;
  void Close() override;
  void SendBinary(const std::vector<uint8_t>& data) override;
  bool IsPollable() override;

  // chime::Runnable overrides
  void Poll() override;

 private:
  // Messages are buffered here first until the socket will accept packets without blocking.
  std::queue<std::vector<uint8_t>> message_queue_;

  // Internal data
  std::vector<uint8_t> received_data_buffer_;

  // This policy sets both connection attempt retry parameters and ping/pong parameters.
  // The retry parameters apply to failures that happen before a websocket connection is established.
  lws_retry_bo_t retry_and_idle_policy_;

  // Count of consecutive connection attempt retries.
  uint16_t connection_retry_count_;

  // Close code provided by server if they initiated hangup.
  uint16_t close_code_;

  // WebsocketConfiguration is provided to the WebsocketFactory to produce the correct Websocket.
  LibwebsocketsWebsocketConfiguration configuration_;

  // Signaling Client listens to these observer methods invoked by the websocket implementation.
  WebsocketObserver* observer_ = nullptr;

  // Info to create the websocket context.
  struct lws_context_creation_info info_ = {};

  // Protocols to use with Libwebsockets (passed by pointer)
  std::unique_ptr<std::vector<struct lws_protocols>> protocols_;

  // Websocket context used to create the websocket instance.
  struct lws_context* context_ = nullptr;

  // Represents the websocket instance.
  struct lws* wsi_ = nullptr;

  // Invoke close event by 
  bool closed_ = false;

  // Needed for Libwebsockets to retry failed connection attempts.
  // lws_sorted_usec_list_t is used by Libwebsockets to stagger connection attempts.
  struct retry_connect {
    lws_sorted_usec_list_t sul;
    LibwebsocketsWebsocket* self;
  } retry_connect_ = {};

  // Invoked by Libwebsockets to retry failed connection attempts.
  static void RetryConnect(lws_sorted_usec_list_t* sul);

  // Invoked by Libwebsockets and provides details to listener about all events, errors, and messages.
  static int Callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

  // Converts from chime::LogLevel to int with flipped bits representing Libwebsockets log levels.
  int ConvertLogLevel(LogLevel level);

  // Log error and notify observer.
  void HandleError(const std::string& error_description);
};

}  // namespace chime

#endif  // SIGNALING_SDK_LIBWEBSOCKETS_WEBSOCKET_H
