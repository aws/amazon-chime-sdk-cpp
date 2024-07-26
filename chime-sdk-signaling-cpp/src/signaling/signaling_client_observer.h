// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_CLIENT_OBSERVER_H_
#define CHIME_SIGNALING_CLIENT_OBSERVER_H_

#include "audio_video/remote_video_source_info.h"
#include "data_message/data_message_error.h"
#include "data_message/data_message_received.h"
#include "session/turn_credentials.h"
#include "signaling/signaling_client_status.h"
#include "signaling/signaling_client_start_info.h"
#include "utils/volume_update.h"
#include "utils/signal_strength_update.h"


#include <map>

namespace chime {
/**
 * Defines observer/callback methods when added by `SignalingClient.AddSignalingClientObserver`
 */
class SignalingClientObserver {
 public:
  virtual ~SignalingClientObserver() = default;
  /**
   * Invoked when sdp answer is available
   * Builders should invoke `peer_connection_->SetRemoteDescription(...)` by creating
   * ```
   * std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, subscribe_ack.sdp_answer(), &error);
   * ```
   *
   * @param sdp_answer - SDP answer received after calling join.
   */
  virtual void OnRemoteDescriptionReceived(const std::string& sdp_answer) = 0;

  // Meeting related audio/video
  /**
   * Invoked when remote video sources are available
   * Builders will need to call `UpdateRemoteVideoSubscriptions` with mid with corresponding
   * attendee id to be able to subscribe to those videos.
   *
   * @param sources - List of `RemoteVideoSourceInfo`s that are available
   */
  virtual void OnRemoteVideoSourcesAvailable(const std::vector<RemoteVideoSourceInfo>& sources){};

  /**
   * Invoked when remote video sources becomes unavailable
   *
   * @param sources - List of `RemoteVideoSource`s that are unavailable
   */
  virtual void OnRemoteVideoSourcesUnavailable(const std::vector<RemoteVideoSourceInfo>& sources){};

  /**
   * Invoked when signaling client has started.
   * @param start_info - Initial information given when started.
   */
  virtual void OnSignalingClientStarted(const SignalingClientStartInfo& start_info) = 0;

  /**
   * Invoked when signaling client has stopped and include status
   * to indicate if it stopped with error or without issue.
   *
   * @param status - Stopped status of `SignalingClient`
   */
  virtual void OnSignalingClientStopped(const SignalingClientStatus& status) = 0;

  // SignalingAttendee Presence
  /**
   * Invoked when an attendee has joined the meeting.
   * A newly joined attendee will receive this callback
   * for attendees that are currently in the meeting.
   *
   * @param attendee - The joined attendee information
   */
  virtual void OnAttendeeJoined(const Attendee& attendee){};

  /**
   * Invoked when an attendee has left the meeting.
   *
   * @param attendee - The left attendee information
   */
  virtual void OnAttendeeLeft(const Attendee& attendee){};

  /**
   * Invoked when an attendee has dropped the meeting due to network.
   *
   * @param attendee - The dropped attendee information
   */
  virtual void OnAttendeeDropped(const Attendee& attendee){};

  // Real time updates volume/signal/data message

  /**
   * Invoked when the updated volumes are available.
   *
   * @param updates - The list of updates, only contains volumes that changed
   */
  virtual void OnVolumeUpdates(const std::vector<VolumeUpdate>& updates){};

  /**
   * Invoked when the updated signal strengths are available.
   *
   * @param updates - The list of updates, only contains signal strength that changed
   */
  virtual void OnSignalStrengthChanges(const std::vector<SignalStrengthUpdate>& updates){};

  /**
   * Invoked when an attendee's audio has muted
   *
   * @param attendee - The muted attendee information
   */
  virtual void OnAttendeeAudioMuted(const Attendee& attendee) {}

  /**
   * Invoked when an attendee's audio has unmuted.
   *
   * @param attendee - The unmuted attendee information
   */
  virtual void OnAttendeeAudioUnmuted(const Attendee& attendee) {}

  /**
   * Invoked when new data messages are received.
   *
   * @param messages - Data messages that contain information about messages sent
   */
  virtual void OnDataMessageReceived(const std::vector<DataMessageReceived>& messages) {}

  /**
   * Invoked when data messages failed to be sent.
   *
   * @param to_send_errors - Data message errors that contain information why it failed.
   */
  virtual void OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) {};
};

}  // namespace chime
#endif  // CHIME_SIGNALING_CLIENT_OBSERVER_H_
