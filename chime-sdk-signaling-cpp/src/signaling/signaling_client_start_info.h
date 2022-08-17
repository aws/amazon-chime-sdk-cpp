// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_JOIN_INFO_H_
#define CHIME_SIGNALING_JOIN_INFO_H_

#include "audio_video/remote_video_source_info.h"
#include "session/turn_credentials.h"

#include <vector>

namespace chime {
/**
 * Join information given when SignalingClientObserver.OnSignalingClientStarted is called.
 */
struct SignalingClientStartInfo {
  /**
   * Turn Credential information needed for peer connection
   *```
   * webrtc::PeerConnectionInterface::RTCConfiguration config;
   * webrtc::PeerConnectionInterface::IceServer server;
   * server.urls = credentials.uris;
   * server.username = credentials.username;
   * server.password = credentials.password;
   * config.servers.push_back(server);
   * ```
   */
  TurnCredentials credentials;
  /**
   * Initial remote video sources. It will be empty if there was no remote video sources.
   */
  std::vector<RemoteVideoSourceInfo> sources;
};
}  // namespace chime
#endif  // CHIME_SIGNALING_JOIN_INFO_H_
