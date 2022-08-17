//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_MOCK_SIGNALING_OBSERVER_H_
#define CHIME_SIGNALING_MOCK_SIGNALING_OBSERVER_H_

#include "gmock/gmock.h"

#include "signaling/signaling_client_observer.h"

using namespace chime;

class MockSignalingClientObserver : public SignalingClientObserver {
 public:
  MOCK_METHOD(void, OnRemoteDescriptionReceived, (const std::string& sdp_answer), (override));
  MOCK_METHOD(void, OnRemoteVideoSourcesAvailable, (const std::vector<RemoteVideoSourceInfo>& sources), (override));
  MOCK_METHOD(void, OnRemoteVideoSourcesUnavailable, (const std::vector<RemoteVideoSourceInfo>& sources), (override));
  MOCK_METHOD(void, OnSignalingClientStarted, (const SignalingClientStartInfo& start_info), (override));
  MOCK_METHOD(void, OnSignalingClientStopped, (const SignalingClientStatus& status), (override));
  MOCK_METHOD(void, OnAttendeeJoined, (const Attendee& attendee), (override));
  MOCK_METHOD(void, OnAttendeeLeft, (const Attendee& attendee), (override));
  MOCK_METHOD(void, OnAttendeeDropped, (const Attendee& attendee), (override));
  MOCK_METHOD(void, OnVolumeUpdates, (const std::vector<VolumeUpdate>& updates), (override));
  MOCK_METHOD(void, OnSignalStrengthChanges, (const std::vector<SignalStrengthUpdate>& updates), (override));
  MOCK_METHOD(void, OnAttendeeAudioMuted, (const Attendee& attendee), (override));
  MOCK_METHOD(void, OnAttendeeAudioUnmuted, (const Attendee& attendee), (override));
  MOCK_METHOD(void, OnDataMessageReceived, (const std::vector<DataMessageReceived>& messages), (override));
};


#endif  // CHIME_SIGNALING_MOCK_SIGNALING_OBSERVER_H_
