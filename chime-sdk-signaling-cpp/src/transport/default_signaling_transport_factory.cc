// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "default_signaling_transport_factory.h"
#include "websocket_signaling_transport.h"
#include "websocket/default_websocket_factory.h"

namespace chime {
std::unique_ptr<SignalingTransport> WebsocketSignalingTransportFactory::CreateSignalingTransport(
    SignalingTransportConfiguration configuration, SignalingTransportObserver* observer) {
  std::unique_ptr<SignalingTransport> transport =
      std::make_unique<WebsocketSignalingTransport>(std::move(configuration), observer, std::move(websocket_factory_));
  return transport;
}

WebsocketSignalingTransportFactory::WebsocketSignalingTransportFactory(
    std::unique_ptr<WebsocketFactory> websocket_factory)
    : websocket_factory_(std::move(websocket_factory)) {}

std::unique_ptr<SignalingTransportFactory> DefaultSignalingTransportFactory::Create() {
  return std::make_unique<WebsocketSignalingTransportFactory>(std::make_unique<DefaultWebsocketFactory>());
}
}  // namespace chime
