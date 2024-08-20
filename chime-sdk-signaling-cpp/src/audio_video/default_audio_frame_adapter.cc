// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#include "utils/logging.h"
#include "signaling/default_signaling_client.h"
#include "default_audio_frame_adapter.h"

#include <math.h>

namespace chime {
// Anonymous namespace. This is only accessible within this file.
namespace {
float NormalizeVolume(uint32_t volume) {
  int32_t db_volume = -volume;
  float normalized = 1.0f - static_cast<float>(db_volume - kSdkMaxVolumeDecibels) / static_cast<float>(kSdkMinVolumeDecibels - kSdkMaxVolumeDecibels);
  float clipped = std::min(std::max(normalized, 0.0f), 1.0f);
  return clipped;
}

bool IsEqual(float x, float y, float epsilon = 1e-6) {
  if (fabs(x - y) < epsilon) {
    return true;
  }
  return false;
}

float NormalizeSignalStrength(uint32_t signal_strength) {
  float normalized = signal_strength / kSdkMaxSignalStrength;
  float clipped = std::min(std::max(normalized, 0.0f), 1.0f);
  return clipped;
}
}

DefaultAudioFrameAdapter::DefaultAudioFrameAdapter(DefaultSignalingClient* default_signaling_client) : default_signaling_client_(default_signaling_client) {}

void DefaultAudioFrameAdapter::OnAudioStreamIdInfo(const signal_sdk::SdkAudioStreamIdInfoFrame &audio_stream_id_info) {
  // Iterate the streams, store the associated attendee data, send out events based on changes.
  // The order is guaranteed, and there's no reuse of the stream id in the same meeting.
  for (const signal_sdk::SdkAudioStreamIdInfo& stream : audio_stream_id_info.streams()) {
    // Skip when no stream id
    if (!stream.has_audio_stream_id()) {
      CHIME_LOG(LogLevel::kDebug, "No audio stream id in audio stream id info frame, will skip.");
      continue;
    }
    bool has_attendee_id = stream.has_attendee_id();
    bool has_external_user_id = stream.has_external_user_id();
    bool has_muted = stream.has_muted();
    bool has_dropped = stream.has_dropped();
    // Handle new attendee
    // Self presence, mute update is not batched, while other attendees' are batched for 1s,
    // and there's no multiple streams for the same stream id in one frame.
    if (has_attendee_id) {
      // Default to empty string if external user id is absent
      std::string external_user_id = has_external_user_id ? stream.external_user_id() : "";
      const auto stream_itr = attendee_id_to_stream_id_.find(stream.attendee_id());
      // Not seen before
      if (stream_itr == attendee_id_to_stream_id_.end()) {
        default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
          observer->OnAttendeeJoined({ stream.attendee_id(), external_user_id });
        });
      }
      attendee_id_to_stream_id_[stream.attendee_id()] = stream.audio_stream_id();
      stream_id_to_attendee_id_[stream.audio_stream_id()] = stream.attendee_id();
      // Audio metadata frame does not carry external user id, we need to store it for query later
      stream_id_to_external_user_id_[stream.audio_stream_id()] = external_user_id;
    }

    // Handle mute state
    if (has_muted) {
      const auto attendee_itr = stream_id_to_attendee_id_.find(stream.audio_stream_id());
      // Skip when stream not seen
      if (attendee_itr == stream_id_to_attendee_id_.end()) {
        CHIME_LOG(LogLevel::kDebug, "Failed to find stream id: " + std::to_string(stream.audio_stream_id()) + " for the mute event, will skip.");
        continue;
      }
      std::string attendee_id = attendee_itr->second;
      std::string external_user_id = stream_id_to_external_user_id_[stream.audio_stream_id()];
      const auto muted_itr = attendee_id_to_muted_.find(attendee_id);
      if (muted_itr == attendee_id_to_muted_.end()) {
        // Not seen before
        if (stream.muted()) {
          default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
            observer->OnAttendeeAudioMuted({ attendee_id, external_user_id });
          });
        }
      } else {
        // Seen before and state changes
        if (muted_itr->second != stream.muted()) {
          default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
            if (stream.muted()) {
              observer->OnAttendeeAudioMuted({ attendee_id, external_user_id });
            } else {
              observer->OnAttendeeAudioUnmuted({ attendee_id, external_user_id });
            }
          });
        }
      }
      // Store the mute state
      attendee_id_to_muted_[attendee_id] = stream.muted();
    }

    // Handle attendee leave
    if (!has_attendee_id && !has_muted) {
      auto attendee_itr = stream_id_to_attendee_id_.find(stream.audio_stream_id());
      if (attendee_itr != stream_id_to_attendee_id_.end()) {
        // Seen the stream before
        std::string attendee_id = attendee_itr->second;
        std::string external_user_id = stream_id_to_external_user_id_[stream.audio_stream_id()];
        // Delete the stream related states
        stream_id_to_attendee_id_.erase(attendee_itr);
        stream_id_to_external_user_id_.erase(stream.audio_stream_id());

        const auto stream_itr = attendee_id_to_stream_id_.find(attendee_id);
        if (stream_itr != attendee_id_to_stream_id_.end()) {
          // Found associated attendee, delete state
          attendee_id_to_stream_id_.erase(attendee_id);
        }
        bool attendee_has_new_stream_id = false;
        for (auto& other_stream : stream_id_to_attendee_id_) {
          if (attendee_id == other_stream.second && other_stream.first > stream.audio_stream_id()) {
            // A new stream id is associated with this attendee
            attendee_has_new_stream_id = true;
            break;
          }
        }
        if (!attendee_has_new_stream_id) {
          // This attendee is absent
          default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
            if (has_dropped && stream.dropped()) {
              observer->OnAttendeeDropped({ attendee_id, external_user_id });
            } else {
              observer->OnAttendeeLeft({ attendee_id, external_user_id });
            }
          });
        }
      }
    }
  }
}

void DefaultAudioFrameAdapter::OnAudioMetadata(const signal_sdk::SdkAudioMetadataFrame &audio_metadata) {
  std::vector<VolumeUpdate> volume_updates;
  std::vector<SignalStrengthUpdate> signal_strength_updates;
  for (const signal_sdk::SdkAudioAttendeeState& state : audio_metadata.attendee_states()) {
    // Skip when no stream id
    if (!state.has_audio_stream_id()) {
      CHIME_LOG(LogLevel::kVerbose, "No audio stream id in audio metadata frame, will skip.")
      continue;
    }
    const auto attendee_itr = stream_id_to_attendee_id_.find(state.audio_stream_id());
    // Skip when attendee not seen
    if (attendee_itr == stream_id_to_attendee_id_.end()) {
      CHIME_LOG(LogLevel::kVerbose, "Failed to find attendee with stream id: " + std::to_string(state.audio_stream_id()) + " for audio metadata, will skip.")
      continue;
    }
    std::string attendee_id = attendee_itr->second;

    // Handle volume
    if (state.has_volume()) {
      Volume new_volume = { NormalizeVolume(state.volume()) };
      const auto volume_itr = attendee_id_to_volume_.find(attendee_id);
      if (volume_itr == attendee_id_to_volume_.end() ||
          !IsEqual(volume_itr->second.normalized_volume, new_volume.normalized_volume)) {
        // Volume changed
        const auto external_user_itr = stream_id_to_external_user_id_.find(state.audio_stream_id());
        std::string external_user_id = external_user_itr != stream_id_to_external_user_id_.end() ?
            external_user_itr->second : "";
        volume_updates.push_back({ { attendee_id, external_user_id }, new_volume });
      }
      // Store current volume
      attendee_id_to_volume_[attendee_id] = new_volume;
    }
    // Handle signal strength
    if (state.has_signal_strength()) {
      SignalStrength new_signal = { NormalizeSignalStrength(state.signal_strength()) };
      const auto signal_itr = attendee_id_to_signal_strength_.find(attendee_id);
      if (signal_itr == attendee_id_to_signal_strength_.end() || !IsEqual(signal_itr->second.normalized_signal_strength, new_signal.normalized_signal_strength)) {
        // Signal changed
        auto external_user_itr = stream_id_to_external_user_id_.find(state.audio_stream_id());
        std::string external_user_id = external_user_itr != stream_id_to_external_user_id_.end() ?
            external_user_itr->second : "";
        signal_strength_updates.push_back({ { attendee_id, external_user_id }, new_signal });
      }
      // Store current signal
      attendee_id_to_signal_strength_[attendee_id] = new_signal;
    }
  }

  // Send updates if any
  if (!volume_updates.empty()) {
    default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
      observer->OnVolumeUpdates(volume_updates);
    });
  }
  if (!signal_strength_updates.empty()) {
    default_signaling_client_->NotifySignalingObserver([&] (SignalingClientObserver* observer) -> void {
      observer->OnSignalStrengthChanges(signal_strength_updates);
    });
  }
}

} // namespace chime
