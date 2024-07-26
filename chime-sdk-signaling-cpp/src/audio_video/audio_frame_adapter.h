// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_AUDIO_FRAME_ADAPTER_H_
#define CHIME_SIGNALING_AUDIO_FRAME_ADAPTER_H_

#include "video_control_sdk.pb.h"


namespace chime {
class AudioFrameAdapter {
 public:
  virtual ~AudioFrameAdapter() = default;
  virtual void OnAudioStreamIdInfo(const signal_sdk::SdkAudioStreamIdInfoFrame& audio_stream_id_info) = 0;
  virtual void OnAudioMetadata(const signal_sdk::SdkAudioMetadataFrame& audio_metadata) = 0;
};

}  // namespace chime
#endif  // CHIME_SIGNALING_AUDIO_FRAME_ADAPTER_H_
