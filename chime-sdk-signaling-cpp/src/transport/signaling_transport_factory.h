// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_TRANSPORT_FACTORY_H_
#define CHIME_SIGNALING_TRANSPORT_FACTORY_H_

#include "signaling_transport.h"
#include "signaling_transport_configuration.h"
#include "websocket/websocket.h"

#include <memory>

namespace chime {
/**
 * Defines factory method for creating `SignalingTransport`. Builders will pass it to
 * `DefaultSignalingDependencies` in order to customize their own transport layer.
 */
class SignalingTransportFactory {
 public:
  virtual ~SignalingTransportFactory() = default;
  virtual std::unique_ptr<SignalingTransport> CreateSignalingTransport(SignalingTransportConfiguration configuration,
                                                                       SignalingTransportObserver* observer) = 0;
};

}  // namespace chime

#endif  // CHIME_SIGNALING_TRANSPORT_FACTORY_H_
