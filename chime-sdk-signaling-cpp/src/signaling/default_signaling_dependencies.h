// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_DEPENDENCIES_H_
#define CHIME_SIGNALING_DEPENDENCIES_H_

#include "transport/signaling_transport_factory.h"

namespace chime {
/**
 * Defines required or optional dependencies for `SignalingClient`.
 * This will be non-copyable
 *
 * NOTE: It will only support websocket implementation.
 */
struct DefaultSignalingDependencies {
  std::unique_ptr<SignalingTransportFactory> signal_transport_factory;
  // Not assignable or copyable
  DefaultSignalingDependencies& operator=(const DefaultSignalingDependencies&) = delete;
  DefaultSignalingDependencies(const DefaultSignalingDependencies&) = delete;

  // It is movable
  DefaultSignalingDependencies& operator=(DefaultSignalingDependencies&&) = default;
  DefaultSignalingDependencies(DefaultSignalingDependencies&&) = default;
  ~DefaultSignalingDependencies() = default;
};
} // namespace chime

#endif  // CHIME_SIGNALING_DEPENDENCIES_H_
