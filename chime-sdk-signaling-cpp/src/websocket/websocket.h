// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

// This file contains the Websocket class and other closely related entities.

#ifndef SIGNALING_SDK_WEBSOCKET_H
#define SIGNALING_SDK_WEBSOCKET_H

#include "utils/runnable.h"
#include "utils/logger/log_level.h"

#include <vector>
#include <string>
#include <map>

namespace chime {

class Websocket : public Runnable {
 public:
  virtual void Connect() = 0;
  virtual void Close() = 0;
  virtual void SendBinary(const std::vector<uint8_t>& data) = 0;
  virtual ~Websocket() = default;
};

struct WebsocketStatus {
  std::string description = "";
};

struct WebsocketErrorStatus : public WebsocketStatus {
  bool retryable = false;
};

struct WebsocketConfiguration {
  std::string protocol_name = "Signaling-SDK";
  std::string url = "";
  std::map<std::string, std::string> additional_headers;
  LogLevel level;
};

class WebsocketObserver {
 public:
  // Receives binary data from server.
  //
  // - Parameter data: Binary data received from server.
  virtual void OnWebsocketBinaryReceived(const std::vector<uint8_t>& data) {}

  // Triggered when a websocket connection has been established.
  virtual void OnWebsocketConnected() {}

  // Triggered when the websocket has closed.
  //
  // - Parameter status: The status of the websocket.
  virtual void OnWebsocketClosed(WebsocketStatus status) {}

  // Triggered when there is an error while trying to make a websocket connection.
  //
  // - Parameter error: The associated websocket error.
  virtual void OnWebsocketError(WebsocketErrorStatus error) {}
  virtual ~WebsocketObserver() = default;
};

class WebsocketFactory {
 public:
  // Creates websockets based on the provided WebsocketConfiguration
  //
  // - Parameter configuration: The configuraton used to decide which websocket to create.
  // - Parameter observer: The observer of websocket errors, events, and messages.
  // - Return: Returns pointer to appropriate websocket.
  virtual std::unique_ptr<Websocket> CreateWebsocket(WebsocketConfiguration configuration,
                                                     WebsocketObserver* observer) = 0;
  virtual ~WebsocketFactory() = default;
};

}  // namespace chime

#endif  // SIGNALING_SDK_WEBSOCKET_H
