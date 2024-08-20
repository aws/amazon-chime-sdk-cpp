// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */
#ifndef CHIME_SIGNALING_VIDEO_CLIENT_AUDIO_FRAME_ADAPTER_H_
#define CHIME_SIGNALING_VIDEO_CLIENT_AUDIO_FRAME_ADAPTER_H_

#include <vector>

#include "audio_frame_adapter.h"
#include "signaling/default_signaling_client.h"
#include "utils/volume.h"
#include "utils/signal_strength.h"

namespace chime {
static constexpr int32_t kSdkMaxVolumeDecibels = -14;
static constexpr int32_t kSdkMinVolumeDecibels = -42;
static constexpr float kSdkMaxSignalStrength = 2.0;

class DefaultAudioFrameAdapter : public AudioFrameAdapter {
 public:
  explicit DefaultAudioFrameAdapter(DefaultSignalingClient* default_signaling_client);

  void OnAudioStreamIdInfo(const signal_sdk::SdkAudioStreamIdInfoFrame& audio_stream_id_info) override;

  void OnAudioMetadata(const signal_sdk::SdkAudioMetadataFrame& audio_metadata) override;

  ~DefaultAudioFrameAdapter() override = default;
 private:
  DefaultSignalingClient* default_signaling_client_;
  std::map<uint32_t, std::string> stream_id_to_attendee_id_;
  std::map<uint32_t, std::string> stream_id_to_external_user_id_;
  std::map<std::string, uint32_t> attendee_id_to_stream_id_;
  std::map<std::string, bool> attendee_id_to_muted_;
  std::map<std::string, Volume> attendee_id_to_volume_;
  std::map<std::string, SignalStrength> attendee_id_to_signal_strength_;
};

}  // namespace chime

#endif // CHIME_SIGNALING_VIDEO_CLIENT_AUDIO_FRAME_ADAPTER_H_
