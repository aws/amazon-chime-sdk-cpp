//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_CLIENT_STATE_H_
#define CHIME_SIGNALING_CLIENT_STATE_H_

namespace chime {

enum class SignalingState {
  kIdle,
  kConnecting,
  kConnected,
  kDisconnecting,
  kDisconnected
};

} // chime
#endif  // CHIME_SIGNALING_CLIENT_STATE_H_
