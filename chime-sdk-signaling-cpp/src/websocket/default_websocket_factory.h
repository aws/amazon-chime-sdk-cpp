// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SIGNALING_SDK_DEFAULT_WEBSOCKET_FACTORY_H
#define SIGNALING_SDK_DEFAULT_WEBSOCKET_FACTORY_H

#include "websocket.h"

#include <memory>

namespace chime {

class DefaultWebsocketFactory : public WebsocketFactory {
 public:
  std::unique_ptr<Websocket> CreateWebsocket(WebsocketConfiguration configuration,
                                             WebsocketObserver* observer) override;
};

}  // namespace chime

#endif  // SIGNALING_SDK_DEFAULT_WEBSOCKET_FACTORY_H
