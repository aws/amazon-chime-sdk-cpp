// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_DEFAULT_SIGNALING_TRANSPORT_FACTORY_H_
#define CHIME_DEFAULT_SIGNALING_TRANSPORT_FACTORY_H_

#include "signaling_transport_factory.h"
#include "websocket/websocket.h"

namespace chime {

/**
 * Websocket implementation of `SignalingTransportFactory`
 */
class WebsocketSignalingTransportFactory : public SignalingTransportFactory {
 public:
  explicit WebsocketSignalingTransportFactory(std::unique_ptr<WebsocketFactory> websocket_factory);
  std::unique_ptr<SignalingTransport> CreateSignalingTransport(SignalingTransportConfiguration configuration,
                                                               SignalingTransportObserver* observer) override;

 private:
  std::unique_ptr<WebsocketFactory> websocket_factory_;
};

class DefaultSignalingTransportFactory {
 public:
  static std::unique_ptr<SignalingTransportFactory> Create();
};

}  // namespace chime

#endif  // CHIME_DEFAULT_SIGNALING_TRANSPORT_FACTORY_H_
