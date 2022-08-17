//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef DATA_MESSAGE_OBSERVER_H_
#define DATA_MESSAGE_OBSERVER_H_

#include "signaling/signaling_client_observer.h"
#include "data_message/data_message_error.h"
#include "data_message/data_message_received.h"

#include "controllers/meeting_controller.h"

#include <vector>

using namespace chime;

class DataMessageObserver : public chime::SignalingClientObserver {
 public:
  void OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) override;
  void OnDataMessageReceived(const std::vector<DataMessageReceived>& messages) override;
  void OnRemoteDescriptionReceived(const std::string& sdp_answer) override {}
  void OnSignalingClientStarted(const SignalingClientStartInfo& start_info) override {}
  void OnSignalingClientStopped(const SignalingClientStatus& status) override {}
};

#endif  // DATA_MESSAGE_OBSERVER_H_
