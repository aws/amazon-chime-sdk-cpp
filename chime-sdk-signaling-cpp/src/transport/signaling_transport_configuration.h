// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#ifndef CHIME_SIGNALING_TRANSPORT_CONFIGURATION_H_
#define CHIME_SIGNALING_TRANSPORT_CONFIGURATION_H_

#include "session/meeting_session_credentials.h"
#include "session/meeting_session_urls.h"

namespace chime {

struct SignalingTransportConfiguration {
  MeetingSessionURLs urls;
  MeetingSessionCredentials credentials;
};

} // namespace chime
#endif  // CHIME_SIGNALING_TRANSPORT_CONFIGURATION_H_
