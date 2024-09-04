// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "libwebsockets_websocket.h"

#include "utils/logger/log_level.h"
#include "websocket/certs/trusted.h"

#include "libwebsockets.h"

#include <string>

namespace chime {

LibwebsocketsWebsocket::LibwebsocketsWebsocket(LibwebsocketsWebsocketConfiguration configuration,
                                               WebsocketObserver* observer)
    : configuration_(configuration), observer_(observer) {
  retry_connect_.self = this;

  // Configure connection retry policy.
  // Table of backoff times used in order of index.
  retry_and_idle_policy_.retry_ms_table = reinterpret_cast<uint32_t*>(&configuration.backoff_ms[0]);

  // Size of the backoff table.
  retry_and_idle_policy_.retry_ms_table_count = LWS_ARRAY_SIZE(configuration.backoff_ms);

  // Number of times to retry before failing.
  retry_and_idle_policy_.conceal_count = LWS_ARRAY_SIZE(configuration.backoff_ms);

  // Percent of artificial random jitter added to retry attempts.
  retry_and_idle_policy_.jitter_percent = configuration.retry_percent_jitter_added;

  // Configure idle policy.
  // Seconds without proof of a valid connection before sending a ping.
  retry_and_idle_policy_.secs_since_valid_ping = configuration.ping_pong_interval_sec;

  // Seconds without proof of a valid connection before hanging up.
  retry_and_idle_policy_.secs_since_valid_hangup =
      configuration.idle_timeout_sec;  // Seconds without proof of a valid connection before hanging up.

  // Configure libwebsocket protocols.
  protocols_ = std::make_unique<std::vector<struct lws_protocols>>(2);
  protocols_->at(0) = {
    configuration.protocol_name.c_str(), Callback,
    0,     // per_session_data_size : Memory Libwebsockets will allocate for user data. NA in our case.
    4096,  // rx_buffer_size        : Max receive buffer size.
    0,     // id                    : Unused by Libwebsockets.
    this,  // user                  : Available as user parameter in Callback
    4096   // tx_buffer_size        : Max send buffer size.
  };
  protocols_->at(1) = LWS_PROTOCOL_LIST_TERM;

  // Setup logging
  int log_level = ConvertLogLevel(configuration.level);
  lws_set_log_level(log_level, NULL);

  // Prepare lws_context_creation_info. This is the context for the client.
  memset(&info_, 0, sizeof info_);
  info_.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
  info_.port = CONTEXT_PORT_NO_LISTEN;  // This is a client, so no need to listen.
  info_.protocols = &protocols_->front();
}

int LibwebsocketsWebsocket::Callback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in,
                                     size_t len) {
  auto* self = static_cast<LibwebsocketsWebsocket*>(user);

  switch (reason) {
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
      const char* error = in ? static_cast<char*>(in) : "(null)";
      self->HandleError(std::string("Error while trying to connect the websocket: ") + std::string(error));

      // Retry connection attempt according to retry policy.
      if (lws_retry_sul_schedule_retry_wsi(wsi, &(self->retry_connect_.sul), self->RetryConnect,
                                           &(self->connection_retry_count_))) {
        lwsl_err("Connection attempts exhausted.\n");
        // Return non-zero to close the connection.
        return -1;
      }
      return 0;
    }

    // Joining already ended meeting will invoke LWS_CALLBACK_CLOSED_CLIENT_HTTP
    case LWS_CALLBACK_CLOSED_CLIENT_HTTP: {
      self->closed_ = true;
      WebsocketStatus status;
      // This will not call `lws_context_destroy` due to it crashing.
      // It could be bug of libwebsocket when creating context leading to undefined behavior
      // if it didn't get created fully before connecting.
      status.description = "Connection already closed";
      self->observer_->OnWebsocketClosed(status);
      self->context_ = nullptr;
      break;
    }

    case LWS_CALLBACK_CLIENT_RECEIVE: {
      lwsl_debug("Data received.\n");
      lwsl_hexdump_debug(in, len);
      const size_t remaining = lws_remaining_packet_payload(wsi);
      auto* uint8_ptr = static_cast<uint8_t*>(in);
      // Messages can be fragmented if the size exceeds max bytes
      // Therefore, it needs to handle fragmented message.
      self->received_data_buffer_.insert(self->received_data_buffer_.end(), uint8_ptr, uint8_ptr + len);
      if (!remaining && lws_is_final_fragment(wsi)) {
        self->observer_->OnWebsocketBinaryReceived(self->received_data_buffer_);
        self->received_data_buffer_.clear();
      }
      break;
    }

    case LWS_CALLBACK_CLIENT_ESTABLISHED: {
      lwsl_info("Handshake complete. Successfully upgraded to websocket.\n");
      self->observer_->OnWebsocketConnected();
      break;
    }

    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: {
      self->close_code_ = *(static_cast<uint16_t*>(in));
      self->closed_ = true;
      lwsl_info("Server initiated connection close. Close code: %hu\n", self->close_code_);
      lwsl_hexdump_debug(in, len);
      // By returning zero, Libwebsockets will echo the close back to the server, then close.
      return 0;
    }

    case LWS_CALLBACK_CLIENT_CLOSED: {
      std::string description;
      if (self->close_code_ != 0) {
        description = std::string("Websocket closed with status: ") + std::to_string(self->close_code_);
      } else {
        description = "Websocket closed.";
      }
      lwsl_info("%s", description.c_str());

      // When connection is closed after websocket connection is established,
      // it doesn't return LWS_CALLBACK_CLIENT_CONNECTION_ERROR, but instead LWS_CALLBACK_CLIENT_CLOSED
      //
      // Looks like context is malformed if it is destroyed by libwebosckets causing memory corruption
      // If it is initiated by application, then safely close, otherwise error it out.
      // TODO: Root cause this in libwebsockets and possibly fix
      //
      // Related: https://github.com/warmcat/libwebsockets/issues/2176
      if (self->closed_) {
        WebsocketStatus status;
        lws_context_destroy(self->context_);
        status.description = description;
        self->observer_->OnWebsocketClosed(status);
      } else {
        // Context has been destroyed by libwebsockets treating as error for 
        // any close that is not requested by us.
        self->HandleError("Connection closed by client");
      } 
      self->context_ = nullptr;
    }

    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: {
      lwsl_info("Appending additional headers.\n");
      unsigned char** data_pointer = static_cast<unsigned char**>(in);
      unsigned char* end_of_data = (*data_pointer) + len;
      for (const auto& additional_header : self->configuration_.additional_headers) {
        if (lws_add_http_header_by_name(wsi, reinterpret_cast<const unsigned char*>(additional_header.first.c_str()),
                                        reinterpret_cast<const unsigned char*>(additional_header.second.c_str()),
                                        additional_header.second.size(), data_pointer, end_of_data)) {
          self->HandleError("Failed to add header: " + additional_header.first + " " + additional_header.second);

          // Return non-zero to close the connection.
          return -1;
        }
      }
      break;
    }
    case LWS_CALLBACK_CLIENT_WRITEABLE: {
      if (self->closed_) return -1;

      // TODO: Check if this can be also fragmented
      if (!self->message_queue_.empty()) {
        std::vector<uint8_t> data = self->message_queue_.front();
        self->message_queue_.pop();

        // Prepend space required by Libwebsockets.
        std::vector<uint8_t> data_with_prefix(LWS_PRE + data.size(), 0);
        std::copy(data.begin(), data.end(), data_with_prefix.begin() + LWS_PRE);

        std::string data_string(data.begin(), data.end());
        lwsl_debug("Writing message with length %lu and data %s \n", data.size(), data_string.c_str());
        int bytes_written = lws_write(self->wsi_, &(data_with_prefix)[LWS_PRE], data.size(), LWS_WRITE_BINARY);
        if (bytes_written == -1) {
          self->HandleError("Fatal write error. Closing.");
          lwsl_hexdump_debug(in, len);
          // Return non-zero to close the connection.
          return -1;
        }
      } else {
        return 0;
      }
      break;
    }
    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP: {
      lwsl_info("Headers complete.");
      break;
    }

    default:
      lwsl_debug("Callback reason, %d, not handled.", static_cast<int>(reason));
      lwsl_hexdump_debug(in, len);
      break;
  }

  // Required by Libwebsockets for internal postprocessing.
  return lws_callback_http_dummy(wsi, reason, user, in, len);
}

void LibwebsocketsWebsocket::RetryConnect(lws_sorted_usec_list_t* sul) {
  // Libwebsockets will offset sul to get its owner struct.
  struct retry_connect* retry = lws_container_of(sul, struct retry_connect, sul);
  LibwebsocketsWebsocket* self = retry->self;
  self->Connect();
}

void LibwebsocketsWebsocket::Connect() {
  // Initialize the client context.
  context_ = lws_create_context(&info_);
  if (!context_) {
    HandleError("lws context initialization failed.");
    return;
  }

  struct lws_client_connect_info connect_info;
  memset(&connect_info, 0, sizeof(connect_info));

  // Parse the url.
  const char* url_temp_path;
  const char* url_protocol;
  std::string url = configuration_.url;
  auto path_start = static_cast<int>(url.find("/control/"));
  std::string path(url.begin() + path_start, url.end());
  if (lws_parse_uri((char*)(url.c_str()), &url_protocol, &connect_info.address, &connect_info.port, &url_temp_path)) {
    HandleError("Could not parse url: " + url);
    return;
  }

  // Set client info and connect the client.
  connect_info.context = context_;
  connect_info.path = path.c_str();
  connect_info.host = connect_info.address;
  connect_info.origin = connect_info.address;
  connect_info.ssl_connection = LCCSCF_USE_SSL;
  connect_info.protocol = configuration_.protocol_name.c_str();
  connect_info.local_protocol_name = configuration_.protocol_name.c_str();
  connect_info.pwsi = &wsi_;
  connect_info.retry_and_idle_policy = &retry_and_idle_policy_;
  connect_info.userdata = this;

  auto lws = lws_client_connect_via_info(&connect_info);
  if (!lws) {
    HandleError("lws_client_connect_via_info failed.");

    // Retry connecting according to the retry policy.
    if (lws_retry_sul_schedule(context_, /* Thread Service Index */ 0, &(retry_connect_.sul), &retry_and_idle_policy_,
                               RetryConnect, &connection_retry_count_)) {
      lwsl_err("Connection attempts exhausted.\n");
      return;
    }
  }

  // TODO: Figure out why it's necessary to manually set SSL certs.
  auto lws_vhost = lws_get_vhost(lws);
  for (const auto& all_prod_cert : all_prod_certs) {
    lws_tls_client_vhost_extra_cert_mem(lws_vhost, all_prod_cert.cert, all_prod_cert.len);
  }
  closed_ = false;
}

bool LibwebsocketsWebsocket::IsPollable() { return true; }

void LibwebsocketsWebsocket::Poll() { if (context_) lws_service(context_, 0); }

void LibwebsocketsWebsocket::Close() {
  if (!closed_) return;
  closed_ = true;
 
  lws_callback_on_writable(wsi_);

  lwsl_user("Closed\n");
}

void LibwebsocketsWebsocket::SendBinary(const std::vector<uint8_t>& data) {
  message_queue_.push(data);

  // Triggers LWS_CALLBACK_CLIENT_WRITEABLE event in Callback when socket is ready to accept data.
  lws_callback_on_writable(wsi_);
}

int LibwebsocketsWebsocket::ConvertLogLevel(LogLevel level) {
  int lws_level = 0;
  switch (level) {
    case LogLevel::kVerbose:
      lws_level = LLL_USER | LLL_NOTICE | LLL_PARSER | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY;
    case LogLevel::kDebug:
      lws_level |= LLL_DEBUG;
    case LogLevel::kInfo:
      lws_level |= LLL_INFO;
    case LogLevel::kWarning:
      lws_level |= LLL_WARN;
    case LogLevel::kError:
      lws_level |= LLL_ERR;
    default:
      lws_level |= LLL_INFO;
      break;
  }
  return lws_level;
}

void LibwebsocketsWebsocket::HandleError(const std::string& error_description) {
  lwsl_err("%s", error_description.c_str());
  WebsocketErrorStatus error_status;
  error_status.description = error_description;
  observer_->OnWebsocketError(error_status);
}

}  // namespace chime
