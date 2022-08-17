// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CONTROLLER_DEPENDENCIES_H_
#define CONTROLLER_DEPENDENCIES_H_

#include "utils/log_sink.h"
#include "observers/session_description_observer.h"

#include "webrtc/api/peer_connection_interface.h"

#include <memory>

using namespace chime;

class SessionDescriptionObserver;

/**
 * Defines dependencies needed by the controller
 */
struct MeetingControllerDependencies {
  // Not assignable or copyable
  MeetingControllerDependencies& operator=(const MeetingControllerDependencies&) = delete;
  MeetingControllerDependencies(const MeetingControllerDependencies&) = delete;

  // It is movable
  MeetingControllerDependencies& operator=(MeetingControllerDependencies&&) = default;
  MeetingControllerDependencies(MeetingControllerDependencies&&) = default;

  std::unique_ptr<SignalingClient> signaling_client;
  std::unique_ptr<rtc::Thread> signaling_thread;
  std::unique_ptr<LogSink> log_sink;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
  SessionDescriptionObserver* session_description_observer = nullptr;
};

#endif  // CONTROLLER_DEPENDENCIES_H_
