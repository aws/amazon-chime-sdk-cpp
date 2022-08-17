// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_CLIENT_H_
#define CHIME_SIGNALING_CLIENT_H_

#include "audio_video/local_audio_configuration.h"
#include "audio_video/local_video_configuration.h"
#include "audio_video/remote_video_source_info.h"
#include "data_message/data_message_to_send.h"
#include "signaling_client_observer.h"
#include "utils/runnable.h"

#include <map>

namespace chime {
/**
 * Defines signaling client that enables builders to connect to
 * Amazon Chime Signaling Server.
 */
class SignalingClient : public Runnable {
 public:
  virtual ~SignalingClient() = default;
  /**
   * Sets local sdp using given sdp.
   *
   * @param sdp - Local sdp updated from webrtc connection.
   */
  virtual void SetLocalDescription(std::string& sdp) = 0;

  /**
   * Sends update to the server. It internally sends new subscribe frame or updates data internally.
   *
   * @return Whether Sending updates were successful.
   */
  virtual bool SendUpdates() = 0;

  /**
   * Adds local video with given local video configuration and mid.
   * Currently, it does not support multiple video
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   * @param configuration - Local video configuration to configure the video
   */
  virtual void AddLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) = 0;

  /**
   * Updates local video with given local video configuration and mid.
   * Currently, it does not support multiple video
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   * @param configuration - Local video configuration to configure the video
   */
  virtual void UpdateLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) = 0;

  /**
   * Removes local video with given mid.
   * Currently, it does not support multiple video
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   */
  virtual void RemoveLocalVideo(const std::string& mid) = 0;

  /**
   * Adds local audio with given local audio configuration and mid.
   * Currently, it does not support multiple audio
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   * @param local_audio_configuration - Local audio configuration for the audio
   */
  virtual void AddLocalAudio(const std::string& mid, const LocalAudioConfiguration& local_audio_configuration) = 0;

  /**
   * Updates local audio with given local audio configuration and mid.
   * Currently, it does not support multiple audio
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   * @param local_audio_configuration - Local audio configuration for the audio
   */
  virtual void UpdateLocalAudio(const std::string& mid, const LocalAudioConfiguration& local_audio_configuration) = 0;

  /**
   * Removes local audio with given mid.
   * Currently, it does not support multiple audio
   *
   * @param mid - [media id](https://developer.mozilla.org/en-US/docs/Web/API/RTCRtpTransceiver/mid) of local video
   * stream from local transceiver
   */
  virtual void RemoveLocalAudio(const std::string& mid) = 0;

  /**
   * Sends data message to signaling server using data channel
   *
   * @param data_message_to_send - Data message to send to the server
   */
  virtual void SendDataMessage(const DataMessageToSend& data_message_to_send) = 0;

  /**
   * Add/Update remote videos.
   * If this is not called, builders will not recieve data from that remote video.
   * The sequence will be processed in add/update and then remove.
   *
   * @param added_updated - Map of mid to newly subscribed or updated `RemoteVideoSourceInfo`s.
   * @param removed - List of mids to be removed from subscribed remote videos
   */
  virtual void UpdateRemoteVideoSubscriptions(const std::map<std::string, RemoteVideoSourceInfo>& added_updated,
                                              const std::vector<std::string>& removed) = 0;

  /**
   * Start `SignalingClient`.
   * In order to get callbacks after start, builders would need to call
   * `AddSignalingClientObserver` before `Start()`
   * It invokes `SignalingClientObserver.OnSignalingClientStarted()`
   */
  virtual void Start() = 0;

  /**
   * Stop `SignalingClient`
   * It invokes `SignalingClientObserver.OnSignalingClientStopped()` when completed
   */
  virtual void Stop() = 0;

  /**
   * Adds `SignalingClientObserver` to `SignalingClient`
   *
   * @param observer - `SignalingClientObserver` to subscribe to signaling events
   */
  virtual void AddSignalingClientObserver(SignalingClientObserver* observer) = 0;

  /**
   * Removes `SignalingClientObserver` from `SignalingClient`
   *
   * @param observer - `SignalingClientObserver` to unsubscribe to signaling events
   */
  virtual void RemoveSignalingClientObserver(SignalingClientObserver* observer) = 0;
};

}  // namespace chime

#endif  // CHIME_SIGNALING_CLIENT_H_
