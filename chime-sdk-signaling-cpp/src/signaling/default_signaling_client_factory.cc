//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "audio_video/default_audio_frame_adapter.h"
#include "default_signaling_client.h"
#include "default_signaling_client_factory.h"
#include "transport/default_signaling_transport_factory.h"
#include "transport/signaling_transport_configuration.h"
#include "utils/logging.h"

namespace chime {

std::unique_ptr<SignalingClient> DefaultSignalingClientFactory::CreateSignalingClient(
    SignalingClientConfiguration configuration, DefaultSignalingDependencies dependencies) {
  CHIME_LOG(LogLevel::kInfo, "Creating DefaultSignalingClient")

  if (dependencies.signal_transport_factory == nullptr) {
    // Use default implementation
    dependencies.signal_transport_factory = DefaultSignalingTransportFactory::Create();
  }

  auto* client = new DefaultSignalingClient(std::move(configuration), std::move(dependencies));
  std::unique_ptr<AudioFrameAdapter> frame_adapter = std::make_unique<DefaultAudioFrameAdapter>(client);
  client->SetAudioFrameAdapter(std::move(frame_adapter));
  std::unique_ptr<SignalingClient> signaling_client = std::unique_ptr<DefaultSignalingClient>(client);

  return signaling_client;
}

}  // namespace chime
