// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "default_websocket_factory.h"
#include "websocket.h"
#include "libwebsockets_websocket.h"

namespace chime {

std::unique_ptr<Websocket> DefaultWebsocketFactory::CreateWebsocket(WebsocketConfiguration configuration,
                                                                    WebsocketObserver* observer) {
  LibwebsocketsWebsocketConfiguration lws_configuration = {};
  lws_configuration.protocol_name = configuration.protocol_name;
  lws_configuration.url = configuration.url;
  lws_configuration.additional_headers = configuration.additional_headers;
  lws_configuration.level = configuration.level;
  return std::make_unique<LibwebsocketsWebsocket>(lws_configuration, observer);
}

}  // namespace chime
