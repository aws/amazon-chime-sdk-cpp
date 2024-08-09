// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_H_
#define CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_H_

#include "audio_video/local_video_configuration.h"
#include "audio_video/local_audio_configuration.h"
#include "audio_video/remote_video_source_info.h"
#include "data_message/data_message_to_send.h"
#include "audio_video/audio_frame_adapter.h"
#include "audio_video/internal_local_stream_configuration.h"
#include "transport/signaling_transport_observer.h"
#include "transport/signaling_transport.h"
#include "audio_video/audio_frame_adapter.h"
#include "signaling/signaling_client.h"
#include "signaling/signaling_client_configuration.h"
#include "signaling/signaling_client_state.h"
#include "default_signaling_dependencies.h"
#include "session/turn_credentials.h"

#include <string>
#include <map>
#include <unordered_map>

namespace chime {

class DefaultSignalingClient : public SignalingClient, public SignalingTransportObserver {
 public:
  ~DefaultSignalingClient() override;
  DefaultSignalingClient(SignalingClientConfiguration signaling_configuration,
                         DefaultSignalingDependencies dependencies);
  bool IsPollable() override;

  void AddSignalingClientObserver(SignalingClientObserver* observer) override;
  void RemoveSignalingClientObserver(SignalingClientObserver* observer) override;

  void StopRun() override;
  void AddLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) override;
  void RemoveLocalVideo(const std::string& mid) override;
  void SetLocalDescription(std::string& sdp) override;
  void UpdateLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) override;

  void AddLocalAudio(const std::string& mid, const LocalAudioConfiguration& local_audio_configuration) override;
  void UpdateLocalAudio(const std::string& mid, const LocalAudioConfiguration &local_audio_configuration) override;
  void RemoveLocalAudio(const std::string& mid) override;

  void SendDataMessage(const DataMessageToSend& data_message_to_send) override;
  void UpdateRemoteVideoSubscriptions(const std::map<std::string, RemoteVideoSourceInfo>& added_updated,
                                      const std::vector<std::string>& removed) override;
  void Start() override;
  void Stop() override;
  void Poll() override;

  bool SendUpdates() override;

  template <typename Function>
  // This is universal reference. It can take both lvalue and rvalue
  void NotifySignalingObserver(Function&& observer_function) {
    for (const auto& observer : observers_) {
      if (observer) observer_function(observer);
    }
  }

  // Observer methods
  void OnSignalFrameReceived(const signal_rtc::SignalFrame& frame) override;
  void OnSignalingConnected() override;
  void OnSignalingErrorReceived(const SignalingError& error) override;
  void OnSignalingClosed(const SignalingCloseEvent& event) override;

  void SetAudioFrameAdapter(std::unique_ptr<AudioFrameAdapter> audio_frame_adapter);

 private:
  void Run() override;
  void SetMute(bool mute);
  bool SendSubscribe();
  bool SendJoin();
  bool SendLeave();

  void Close();

  // Handle frame
  void UpdateTurnCredentials(const signal_rtc::JoinAckFrame& join_ack);
  bool TurnCredentialsExpired();
  void HandleIndexFrame(const signal_rtc::IndexFrame& index_frame);
  void HandleSubAckFrame(const signal_rtc::SubscribeAckFrame& subscribe_ack_frame);
  void HandleDataMessageFrame(const signal_rtc::DataMessageFrame& data_message_frame);

  std::unique_ptr<SignalingTransport> signaling_transport_;
  std::unique_ptr<AudioFrameAdapter> audio_frame_adapter_;
  std::vector<SignalingClientObserver*> observers_;
  std::string sdp_;
  SignalingState state_ = SignalingState::kIdle;
  SignalingClientConfiguration signaling_configuration_;
  std::map<std::string, InternalStreamConfiguration> local_video_sources_;
  std::map<std::string, InternalStreamConfiguration> local_audio_sources_;
  std::map<std::string/* mid */, InternalStreamConfiguration> remote_video_sources_;

  std::map<std::string /* attendee_id */, InternalStreamConfiguration> attendee_id_to_internal_configurations_;

  bool is_muted_ = false;
  bool has_received_first_index_ = false;
  std::chrono::system_clock::time_point turn_credentials_expire_time_;
  bool is_joined_ = false;
  TurnCredentials turn_credentials_ {};
};

}  // namespace chime

#endif  // CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_H_
