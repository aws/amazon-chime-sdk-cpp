// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#include "default_signaling_client.h"

#include "audio_video/internal_local_stream_configuration.h"
#include "audio_video/media_section.h"
#include "data_message/data_message_received.h"
#include "data_message/data_message_to_send.h"
#include "transport/websocket_signaling_transport.h"
#include "utils/sdp_utils.h"
#include "utils/logging.h"
#include "default_signaling_dependencies.h"
#include "utils/signal_frame_debug_utils.h"
#include "version.h"

#include <utility>

namespace chime {

// Anonymous namespace. This is only accessible within this file.
// All the helper function is defined here to avoid any future name collison
namespace {
const std::regex kTopicRegex{"^[a-zA-Z0-9_-]{1,36}$"};
constexpr int kMaxSize = 2048;
bool IsLocal(MediaDirection direction) {
  return direction == MediaDirection::kInactive || direction == MediaDirection::kSendOnly ||
         direction == MediaDirection::kSendRecv;
}
bool IsSending(MediaDirection direction) {
  return direction == MediaDirection::kSendOnly || direction == MediaDirection::kSendRecv;
}

DataMessageSendError BuildDataMessageError(const DataMessageToSend& data_message_to_send,
                                           DataMessageSendErrorReason reason) {
  DataMessageSendError data_message_send_error{{data_message_to_send.topic, data_message_to_send.data}, reason};

  return data_message_send_error;
}

void AddAttendee(InternalStreamConfiguration& internal_stream_configuration,
                 const SignalingClientConfiguration& signaling_configuration) {
  internal_stream_configuration.attendee.attendee_id =
      signaling_configuration.meeting_configuration.credentials.attendee_id;
  internal_stream_configuration.attendee.external_user_id =
      signaling_configuration.meeting_configuration.credentials.external_user_id;
}
}  // namespace

void DefaultSignalingClient::SetLocalDescription(std::string& sdp) {
  sdp_ = sdp;
  CHIME_LOG(LogLevel::kDebug, "SDP set: " + sdp)
}

void DefaultSignalingClient::UpdateLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) {
  bool is_exists = local_video_sources_.find(mid) != local_video_sources_.end();
  // It will create internal_local_video_configuration if it is not in the map
  // Even though we have mapping, it will be only one video currently
  InternalStreamConfiguration& internal_local_video_configuration = local_video_sources_[mid];
  internal_local_video_configuration.max_bitrate_kbps = configuration.max_bitrate_kbps;
  CHIME_LOG(LogLevel::kInfo, "Set bitrate to " + std::to_string(internal_local_video_configuration.max_bitrate_kbps));
  if (!is_exists) {
    AddAttendee(internal_local_video_configuration, signaling_configuration_);
  }
}

void DefaultSignalingClient::UpdateLocalAudio(const std::string& mid,
                                              const LocalAudioConfiguration& local_audio_configuration) {
  // TODO: Later, this should support multiple audio
  if (local_audio_sources_.find(mid) == local_audio_sources_.end()) {
    InternalStreamConfiguration internal_local_audio_configuration;
    local_audio_sources_[mid] = internal_local_audio_configuration;
  }
    SetMute(local_audio_configuration.mute_state == MuteState::kMute);
}

void DefaultSignalingClient::SendDataMessage(const DataMessageToSend& data_message_to_send) {
  if (state_ != SignalingState::kConnected) {
    return;
  }

  std::vector<DataMessageSendError> data_message_send_errors;

  if (data_message_to_send.data.size() > kMaxSize) {
    data_message_send_errors.push_back(
        BuildDataMessageError(data_message_to_send, DataMessageSendErrorReason::kInvalidDataMessageSize));
  } else if (!std::regex_match(data_message_to_send.topic, kTopicRegex)) {
    data_message_send_errors.push_back(
        BuildDataMessageError(data_message_to_send, DataMessageSendErrorReason::kInvalidTopic));
  } else if (data_message_to_send.lifetime_ms < 0) {
    data_message_send_errors.push_back(
        BuildDataMessageError(data_message_to_send, DataMessageSendErrorReason::kInvalidLifeTimeMs));
  }

  if (!data_message_send_errors.empty()) {
    NotifySignalingObserver([&data_message_send_errors](SignalingClientObserver* observer) -> void {
      observer->OnDataMessagesFailedToSend(data_message_send_errors);
    });
    return;
  }

  signal_rtc::SignalFrame signal_frame;
  signal_frame.set_type(signal_rtc::SignalFrame_Type_DATA_MESSAGE);
  signal_rtc::DataMessageFrame* data_message_frame = signal_frame.mutable_data_message();
  signal_rtc::DataMessagePayload message_payload = signal_rtc::DataMessagePayload();
  message_payload.set_topic(data_message_to_send.topic);
  message_payload.set_data(data_message_to_send.data);
  message_payload.set_lifetime_ms(data_message_to_send.lifetime_ms);
  *data_message_frame->add_messages() = message_payload;

  signaling_transport_->SendSignalFrame(signal_frame);
}

void DefaultSignalingClient::Start() {
  CHIME_LOG(LogLevel::kInfo, "Starting DefaultSignalingClient")
  // TODO @hokyungh: check if cycle dependency can have memory leak.
  state_ = SignalingState::kConnecting;
  signaling_transport_->Start();
}

void DefaultSignalingClient::Close() {
  signaling_transport_->Stop();
  local_video_sources_.clear();
  local_audio_sources_.clear();
  remote_video_sources_.clear();
  attendee_id_to_internal_configurations_.clear();
}

void DefaultSignalingClient::Stop() {
  CHIME_LOG(LogLevel::kInfo, "Stopping DefaultSignalingClient")
  if (state_ != SignalingState::kConnected) return;
  // Gracefully, shutting down if it is connected. It will call Close() to terminate when LEAVE_ACK is received.
  SendLeave();
  state_ = SignalingState::kDisconnecting;
}

void DefaultSignalingClient::Poll() { signaling_transport_->Poll(); }

void DefaultSignalingClient::UpdateRemoteVideoSubscriptions(
    const std::map<std::string, RemoteVideoSourceInfo>& added_updated, const std::vector<std::string>& removed) {
  for (const auto& added : added_updated) {
    auto it = remote_video_sources_.find(added.first);
    auto attendee_to_internal_stream = attendee_id_to_internal_configurations_.find(added.second.attendee.attendee_id);
    if (it == remote_video_sources_.end()) {
      if (attendee_to_internal_stream == attendee_id_to_internal_configurations_.end()) {
        // should not reach this point since builders should call it after OnRemoteVideosAvailable
        CHIME_LOG(LogLevel::kWarning, "UpdateRemoteVideoSubscriptions seems to be called before OnRemoteVideoAvailable")
      } else {
        remote_video_sources_[added.first] = attendee_to_internal_stream->second;
      }
    } else {
      if (attendee_to_internal_stream == attendee_id_to_internal_configurations_.end()) {
        // should not reach this point since builders should call it after OnRemoteVideoAvailable
        CHIME_LOG(LogLevel::kWarning, "UpdateRemoteVideoSubscriptions seems to be called before OnRemoteVideoAvailable")
      } else {
        // just update
        it->second.group_id = attendee_to_internal_stream->second.group_id;
        it->second.max_bitrate_kbps = added.second.max_bitrate_kbps;
      }
    }
  }

  for (const auto& removed_subscription : removed) {
    remote_video_sources_.erase(removed_subscription);
  }
}

bool DefaultSignalingClient::SendSubscribe() {
  if (state_ != SignalingState::kConnected) return false;
  CHIME_LOG(LogLevel::kDebug, "Sending subscribe")
  signal_rtc::StreamServiceType duplex = signal_rtc::StreamServiceType::RX;
  signal_rtc::SignalFrame signal_frame;
  signal_frame.set_type(signal_rtc::SignalFrame_Type_SUBSCRIBE);
  signal_rtc::SubscribeFrame* subscribe_frame = signal_frame.mutable_sub();
  subscribe_frame->set_sdp_offer(sdp_);

  std::vector<MediaSection> media_sections = SDPUtils::ParseSDP(sdp_);

  std::vector<uint32_t> receive_streams;
  std::vector<signal_rtc::StreamDescriptor> stream_descriptors;
  // This is to make sure we have each sdp section in order
  for (const auto& media_section : media_sections) {
    if (media_section.type == MediaType::kAudio) {
      CHIME_LOG(LogLevel::kDebug, "Current mid " + media_section.mid)
      if (local_audio_sources_.find(media_section.mid) != local_audio_sources_.end()) {
        // No mid found, so we just need to have 0 for it.
        InternalStreamConfiguration& internal_local_stream_configuration = local_audio_sources_[media_section.mid];
        // For audio, needs to add 0 since this is what server side expects.
        receive_streams.push_back(0);
        signal_rtc::StreamDescriptor stream_descriptor;
        stream_descriptor.set_stream_id(internal_local_stream_configuration.stream_id);
        stream_descriptor.set_group_id(internal_local_stream_configuration.group_id);
        stream_descriptor.set_max_bitrate_kbps(internal_local_stream_configuration.max_bitrate_kbps);
        stream_descriptors.emplace_back(stream_descriptor);
      }
    } else {
      if (IsLocal(media_section.direction) && IsSending(media_section.direction)) {
        duplex = signal_rtc::StreamServiceType::DUPLEX;
      }
      // Check if local mid is there. We do not need to subscribe for local video on receive side.
      if (local_video_sources_.find(media_section.mid) != local_video_sources_.end()) {
        InternalStreamConfiguration& internal_local_stream_configuration = local_video_sources_[media_section.mid];
        // One of config contains send.
        signal_rtc::StreamDescriptor stream_descriptor;
        stream_descriptor.set_stream_id(internal_local_stream_configuration.stream_id);
        stream_descriptor.set_group_id(internal_local_stream_configuration.group_id);
        stream_descriptor.set_max_bitrate_kbps(internal_local_stream_configuration.max_bitrate_kbps);
        stream_descriptors.emplace_back(stream_descriptor);
      } else {
        // Should be remote videos
        // We'll filter out only those that builders wanted to subscribe which is called by
        // UpdateRemoteVideoSubscriptions
        auto it = remote_video_sources_.find(media_section.mid);
        if (it == remote_video_sources_.end()) continue;

        if (media_section.direction == MediaDirection::kInactive) {
          // Following same behavior as video_client_impl
          receive_streams.push_back(0);
        } else {
          receive_streams.push_back(it->second.stream_id);
        }
      }
    }
  }

  if (duplex == signal_rtc::DUPLEX || duplex == signal_rtc::TX) {
    for (const auto& stream_descriptor : stream_descriptors) {
      *subscribe_frame->add_send_streams() = stream_descriptor;
    }
  }

  subscribe_frame->set_duplex(duplex);

  *subscribe_frame->mutable_receive_stream_ids() = {receive_streams.begin(), receive_streams.end()};

  if (!signaling_configuration_.meeting_configuration.urls.audio_host_url.empty()) {
    subscribe_frame->set_xrp_host(signaling_configuration_.meeting_configuration.urls.audio_host_url);
  }

  if (is_muted_) subscribe_frame->set_xrp_muted(is_muted_);

  CHIME_LOG(LogLevel::kDebug, SignalFrameDebugUtils::SubscribeFrameDebugString(*subscribe_frame))

  return signaling_transport_->SendSignalFrame(signal_frame);
}

bool DefaultSignalingClient::SendJoin() {
  CHIME_LOG(LogLevel::kInfo, "Sending Join event")
  signal_rtc::SignalFrame signal_frame;
  signal_frame.set_type(signal_rtc::SignalFrame_Type_JOIN);

  signal_rtc::JoinFrame* join_frame = signal_frame.mutable_join();
  // This has to be 2
  join_frame->set_protocol_version(2);
  join_frame->set_max_num_of_videos(25);

  // Add little bit more client information
  signal_rtc::ClientDetails* client_details = join_frame->mutable_client_details();
  client_details->set_client_source("amazon-chime-sdk-cpp");
  client_details->set_chime_sdk_version(PROJECT_VERSION);

  // HAS_STREAM_UPDATE is needed for attendee presence
  uint32_t flags = signal_rtc::EXCLUDE_SELF_CONTENT_IN_INDEX;
  if (signaling_configuration_.enable_attendee_update) flags |= signal_rtc::HAS_STREAM_UPDATE;
  join_frame->set_flags(flags);

  return signaling_transport_->SendSignalFrame(signal_frame);
}

void DefaultSignalingClient::OnSignalFrameReceived(const signal_rtc::SignalFrame& frame) {
  switch (frame.type()) {
    case signal_rtc::SignalFrame::JOIN_ACK:
      CHIME_LOG(LogLevel::kInfo, "Join is successful")
      UpdateTurnCredentials(frame.joinack());
      break;
    case signal_rtc::SignalFrame::LEAVE_ACK:
      CHIME_LOG(LogLevel::kInfo, "Leave is successful")
      state_ = SignalingState::kDisconnected;
      Close();
      break;
    case signal_rtc::SignalFrame::INDEX:
      HandleIndexFrame(frame.index());
      break;
    case signal_rtc::SignalFrame::SUBSCRIBE_ACK:
      HandleSubAckFrame(frame.suback());
      break;
    case signal_rtc::SignalFrame::BITRATES:
      // TODO @hokyungh: implement it.
      break;
    case signal_rtc::SignalFrame::AUDIO_METADATA:
      if (audio_frame_adapter_) audio_frame_adapter_->OnAudioMetadata(frame.audio_metadata());
      break;
    case signal_rtc::SignalFrame::AUDIO_STREAM_ID_INFO:
      if (audio_frame_adapter_) audio_frame_adapter_->OnAudioStreamIdInfo(frame.audio_stream_id_info());
      break;
    case signal_rtc::SignalFrame::DATA_MESSAGE:
      HandleDataMessageFrame(frame.data_message());
      break;
    default:
      // Unexpected type
      CHIME_LOG(LogLevel::kDebug, "Received unexpected signaling type: " + std::to_string(frame.type()))
      break;
  }
}

void DefaultSignalingClient::OnSignalingConnected() {
  CHIME_LOG(LogLevel::kInfo, "OnSignalingConnected")
  state_ = SignalingState::kConnected;
  SendJoin();
}

void DefaultSignalingClient::OnSignalingErrorReceived(const SignalingError& error) {
  CHIME_LOG(LogLevel::kError, "Signaling has received error " + error.description)

  state_ = SignalingState::kDisconnected;

  is_joined_ = false;
  SignalingClientStatus status;
  status.type = SignalingClientStatusType::kClientError;
  status.reason = error.description;

  NotifySignalingObserver(
      [&status](SignalingClientObserver* observer) -> void { observer->OnSignalingClientStopped(status); });
}

void DefaultSignalingClient::OnSignalingClosed(const SignalingCloseEvent& event) {
  CHIME_LOG(LogLevel::kInfo, "Signaling has closed")
  state_ = SignalingState::kDisconnected;
  is_joined_ = false;
  SignalingClientStatus status;
  status.type = SignalingClientStatusType::kOk;
  has_received_first_index_ = false;
  NotifySignalingObserver(
      [&status](SignalingClientObserver* observer) -> void { observer->OnSignalingClientStopped(status); });
}

bool DefaultSignalingClient::SendUpdates() { return SendSubscribe(); }

void DefaultSignalingClient::AddSignalingClientObserver(SignalingClientObserver* observer) {
  observers_.push_back(observer);
}

void DefaultSignalingClient::RemoveSignalingClientObserver(SignalingClientObserver* observer) {
  auto observer_found = std::find(observers_.cbegin(), observers_.cend(), observer);
  if (observer_found == observers_.cend()) return;
  observers_.erase(observer_found);
}

void DefaultSignalingClient::UpdateTurnCredentials(const signal_rtc::JoinAckFrame& join_ack) {
  if (!join_ack.has_turn_credentials()) {
    CHIME_LOG(LogLevel::kError, "No turn credential exist")
    return;
  }

  const signal_rtc::TurnCredentials& turn_creds = join_ack.turn_credentials();

  if (!turn_creds.has_username() || !turn_creds.has_password() || !turn_creds.has_ttl() ||
      turn_creds.uris_size() == 0) {
    CHIME_LOG(LogLevel::kError, "turn credential exist, but components missing")
    return;
  }

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::seconds ttl(turn_creds.ttl());
  turn_credentials_expire_time_ = now + ttl;

  std::vector<std::string> uris;
  for (const auto& uri : turn_creds.uris()) {
    uris.push_back(uri);
  }
  turn_credentials_.uris = uris;
  turn_credentials_.username = turn_creds.username();
  turn_credentials_.password = turn_creds.password();
  turn_credentials_.ttl = turn_creds.ttl();
  is_joined_ = true;
}

void DefaultSignalingClient::HandleIndexFrame(const signal_rtc::IndexFrame& index_frame) {
  if (state_ != SignalingState::kConnected || !is_joined_) {
    CHIME_LOG(LogLevel::kWarning, "Should not receive index frame before join")
    return;
  }

  if (index_frame.has_at_capacity() && index_frame.at_capacity()) {
    // TODO @hokyungh: handle capacity
    CHIME_LOG(LogLevel::kWarning, "It reached video max capacity")
    return;
  }

  CHIME_LOG(LogLevel::kDebug, SignalFrameDebugUtils::IndexFrameDebugString(index_frame))

  std::vector<RemoteVideoSourceInfo> removed;
  for (const auto& pair : attendee_id_to_internal_configurations_) {
    auto it = std::find_if(index_frame.sources().begin(), index_frame.sources().end(),
                           [&](const signal_rtc::StreamDescriptor& stream) {
                             return pair.second.attendee.attendee_id == stream.profile_uuid();
                           });
    // previous index does not exist anymore, so added to remove list
    if (it == index_frame.sources().end()) {
      RemoteVideoSourceInfo remote_video_source;
      remote_video_source.max_bitrate_kbps = pair.second.max_bitrate_kbps;
      remote_video_source.attendee.attendee_id = pair.second.attendee.attendee_id;
      remote_video_source.attendee.external_user_id = pair.second.attendee.external_user_id;
      removed.push_back(remote_video_source);
    }
  }

  for (const auto& remote_video_source : removed) {
    attendee_id_to_internal_configurations_.erase(remote_video_source.attendee.attendee_id);
  }

  std::vector<RemoteVideoSourceInfo> sources;
  for (const signal_rtc::StreamDescriptor& stream : index_frame.sources()) {
    auto attend_id_to_internal_config_it = attendee_id_to_internal_configurations_.find(stream.profile_uuid());
    if (attend_id_to_internal_config_it == attendee_id_to_internal_configurations_.end()) {
      // Sets internal configuration for local mapping
      InternalStreamConfiguration remote_configuration;
      RemoteVideoSourceInfo remote_video_source;
      remote_configuration.stream_id = stream.stream_id();
      remote_configuration.group_id = stream.group_id();
      remote_configuration.attendee.attendee_id = stream.profile_uuid();
      remote_configuration.attendee.external_user_id = stream.external_user_id();
      remote_configuration.max_bitrate_kbps = stream.max_bitrate_kbps();

      attendee_id_to_internal_configurations_[stream.profile_uuid()] = remote_configuration;

      // Sets remote video sources as well
      remote_video_source.attendee.attendee_id = stream.profile_uuid();
      remote_video_source.attendee.external_user_id = stream.external_user_id();
      remote_video_source.max_bitrate_kbps = stream.max_bitrate_kbps();
      remote_video_source.stream_id = stream.stream_id();

      sources.push_back(remote_video_source);
    } else {
      // Just updates value from the stream
      attend_id_to_internal_config_it->second.max_bitrate_kbps = stream.max_bitrate_kbps();
      attend_id_to_internal_config_it->second.attendee.attendee_id = stream.profile_uuid();
      attend_id_to_internal_config_it->second.attendee.external_user_id = stream.external_user_id();
      attend_id_to_internal_config_it->second.group_id = stream.group_id();
    }
  }

  if (!has_received_first_index_) {
    has_received_first_index_ = true;
    
    if (TurnCredentialsExpired()) {
      CHIME_LOG(LogLevel::kError, "TURN credentials expired.")
      return;
    }
    TurnCredentials credentials = turn_credentials_;

    NotifySignalingObserver([&credentials, &sources](SignalingClientObserver* observer) -> void {
      observer->OnSignalingClientStarted({credentials, sources});
    });

    // No need to call OnRemoteVideoSourcesAvailable or OnRemoteVideoSourcesUnavailable for the first time
    return;
  }

  if (!sources.empty()) {
    NotifySignalingObserver(
        [&sources](SignalingClientObserver* observer) -> void { observer->OnRemoteVideoSourcesAvailable(sources); });
  }

  if (!removed.empty()) {
    NotifySignalingObserver(
        [&removed](SignalingClientObserver* observer) -> void { observer->OnRemoteVideoSourcesUnavailable(removed); });
  }
}

DefaultSignalingClient::~DefaultSignalingClient() {
  Stop();
}

DefaultSignalingClient::DefaultSignalingClient(SignalingClientConfiguration signaling_configuration,
                                               DefaultSignalingDependencies dependencies) {
  signaling_configuration_ = std::move(signaling_configuration);
  SignalingTransportConfiguration config{};
  config.credentials = signaling_configuration_.meeting_configuration.credentials;
  config.urls = signaling_configuration_.meeting_configuration.urls;
  signaling_transport_ = dependencies.signal_transport_factory->CreateSignalingTransport(config, this);
  is_muted_ = signaling_configuration_.mute_on_join;
}

std::string subscribeAckFrameDebugString(const signal_rtc::SubscribeAckFrame& subAck) {
  std::string str = "SignalFrame:\nSubscribeAck: \nduplex: " + SignalFrameDebugUtils::DuplexToString(subAck.duplex()) +
                    "\nallocations:{";

  for (const signal_rtc::StreamAllocation& allocation : subAck.allocations()) {
    str += "\n\t";
    if (allocation.has_group_id()) {
      str += "group_id:" + std::to_string(allocation.group_id()) + " ";
    }
    if (allocation.has_stream_id()) {
      str += "stream_id: " + std::to_string(allocation.stream_id()) + " ";
    }
    if (allocation.has_track_label()) {
      str += "track_label:" + allocation.track_label() + " ";
    }
  }
  str += "\n}\n";

  str += "tracks:{";
  for (const signal_rtc::TrackMapping& mapping : subAck.tracks()) {
    str += "\n\t";
    if (mapping.has_stream_id()) {
      str += "stream_id:" + std::to_string(mapping.stream_id()) + " ";
    }
    if (mapping.has_ssrc()) {
      str += "ssrc:" + std::to_string(mapping.ssrc()) + " ";
    }
    if (mapping.has_track_label()) {
      str += "\ttrack_label: " + mapping.track_label() + " ";
    }
  }
  str += "\n}\n";
  if (subAck.has_sdp_answer()) {
    str += "SDP:\n" + subAck.sdp_answer();
  }
  return str;
}

void DefaultSignalingClient::HandleSubAckFrame(const signal_rtc::SubscribeAckFrame& subscribe_ack_frame) {
  if (!subscribe_ack_frame.has_sdp_answer()) {
    CHIME_LOG(LogLevel::kError, "Empty sdp offer received.")
    return;
  }

  const std::string& sdp_answer = subscribe_ack_frame.sdp_answer();
  CHIME_LOG(LogLevel::kDebug, "Received sdp offer: " + subscribeAckFrameDebugString(subscribe_ack_frame))

  std::vector<MediaSection> media_sections = SDPUtils::ParseSDP(subscribe_ack_frame.sdp_answer());

  int allocationInd = 0;
  // TODO @hokyungh: This is best try on mapping between local and mid.
  // It might be best if server also gives mid so that we don't have to manage this mapping
  for (const auto& media_section : media_sections) {
    if (allocationInd >= subscribe_ack_frame.allocations_size() ||
        allocationInd >= (local_audio_sources_.size() + local_video_sources_.size())) {
      break;
    }
    const signal_rtc::StreamAllocation& allocation = subscribe_ack_frame.allocations()[allocationInd];
    if (local_audio_sources_.find(media_section.mid) != local_audio_sources_.end()) {
      local_audio_sources_[media_section.mid].group_id = allocation.group_id();
      local_audio_sources_[media_section.mid].stream_id = allocation.stream_id();
      allocationInd++;
    } else if (local_video_sources_.find(media_section.mid) != local_video_sources_.end()) {
      local_video_sources_[media_section.mid].group_id = allocation.group_id();
      local_video_sources_[media_section.mid].stream_id = allocation.stream_id();
      allocationInd++;
    }
  }

  NotifySignalingObserver(
      [&sdp_answer](SignalingClientObserver* observer) -> void { observer->OnRemoteDescriptionReceived(sdp_answer); });
}

void DefaultSignalingClient::HandleDataMessageFrame(const signal_rtc::DataMessageFrame& data_message_frame) {
  if (data_message_frame.messages_size() <= 0) return;
  std::vector<DataMessageReceived> messages;
  std::vector<DataMessageSendError> error_messages;
  messages.reserve(data_message_frame.messages_size());

  for (int index = 0; index < data_message_frame.messages_size(); index++) {
    // It is throttled, we should invoke an error
    if (data_message_frame.messages(index).ingest_time_ns() == 0) {
      DataMessageSendError data_message_send_error;
      data_message_send_error.data_message.data = data_message_frame.messages(index).data();
      data_message_send_error.data_message.topic = data_message_frame.messages(index).topic();
      data_message_send_error.reason = DataMessageSendErrorReason::kThrottled;
      error_messages.push_back(data_message_send_error);
    } else {
      DataMessageReceived message = {};
      message.timestamp_ms = data_message_frame.messages(index).ingest_time_ns() / 1000000;
      message.topic = data_message_frame.messages(index).topic();
      message.data = data_message_frame.messages(index).data();
      message.attendee.attendee_id = data_message_frame.messages(index).sender_profile_id();
      message.attendee.external_user_id = data_message_frame.messages(index).sender_external_user_id();
      messages.push_back(message);
    }
  }

  if (!messages.empty()) {
    NotifySignalingObserver(
        [&messages](SignalingClientObserver* observer) -> void { observer->OnDataMessageReceived(messages); });
  }

  if (!error_messages.empty()) {
    NotifySignalingObserver([&error_messages](SignalingClientObserver* observer) -> void {
      observer->OnDataMessagesFailedToSend(error_messages);
    });
  }
}

void DefaultSignalingClient::SetMute(bool mute) {
  CHIME_LOG(LogLevel::kDebug, "Setting mute from " + std::to_string(is_muted_) + " to " + std::to_string(mute))
  if (is_muted_ == mute) return;
  is_muted_ = mute;
  if (state_ != SignalingState::kConnected) {
    CHIME_LOG(LogLevel::kWarning, "Cannot send mute signal in current state; Not connected")
    return;
  }

  signal_rtc::SignalFrame signal_frame;
  signal_frame.set_type(signal_rtc::SignalFrame_Type_AUDIO_CONTROL);
  signal_rtc::AudioControlFrame* audio_control_frame = signal_frame.mutable_audio_control();
  audio_control_frame->set_muted(mute);

  signaling_transport_->SendSignalFrame(signal_frame);
}

void DefaultSignalingClient::SetAudioFrameAdapter(std::unique_ptr<AudioFrameAdapter> audio_frame_adapter) {
  audio_frame_adapter_ = std::move(audio_frame_adapter);
}

void DefaultSignalingClient::Run() { signaling_transport_->Run(); }
bool DefaultSignalingClient::IsPollable() { return signaling_transport_->IsPollable(); }
void DefaultSignalingClient::StopRun() { signaling_transport_->StopRun(); }
void DefaultSignalingClient::AddLocalVideo(const std::string& mid, const LocalVideoConfiguration& configuration) {
  UpdateLocalVideo(mid, configuration);
}
void DefaultSignalingClient::RemoveLocalVideo(const std::string& mid) { local_video_sources_.erase(mid); }
void DefaultSignalingClient::AddLocalAudio(const std::string& mid,
                                           const LocalAudioConfiguration& local_audio_configuration) {
  UpdateLocalAudio(mid, local_audio_configuration);
}
void DefaultSignalingClient::RemoveLocalAudio(const std::string& mid) { local_audio_sources_.erase(mid); }

bool DefaultSignalingClient::SendLeave() {
  signal_rtc::SignalFrame signal_frame;
  signal_frame.set_type(signal_rtc::SignalFrame_Type_LEAVE);
  return signaling_transport_->SendSignalFrame(signal_frame);
}

bool DefaultSignalingClient::TurnCredentialsExpired() {
  auto now = std::chrono::system_clock::now();
  return ((now - turn_credentials_expire_time_).count() > 0);
}

}  // namespace chime
