//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_FACTORY_H_
#define CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_FACTORY_H_

#include "signaling/default_signaling_dependencies.h"
#include "signaling/signaling_client.h"
#include "signaling/signaling_client_configuration.h"

#include <memory>

namespace chime {

class DefaultSignalingClientFactory {
 public:
  static std::unique_ptr<SignalingClient> CreateSignalingClient(SignalingClientConfiguration configuration,
                                                 DefaultSignalingDependencies dependencies);
};

} // namespace chime

#endif //CHIME_SIGNALING_DEFAULT_SIGNALING_CLIENT_FACTORY_H_
