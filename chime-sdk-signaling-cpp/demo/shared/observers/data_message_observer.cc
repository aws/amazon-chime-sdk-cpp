//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "data_message_observer.h"

#include "data_message/data_message_error.h"
#include "data_message/data_message_received.h"

#include "webrtc/rtc_base/logging.h"

#include <vector>

using namespace chime;

void DataMessageObserver::OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) {
  for (const auto& error : to_send_errors) {
    RTC_LOG(LS_ERROR) << "Data message failed to send. Error: " << error.DebugString();
  }
}

void DataMessageObserver::OnDataMessageReceived(const std::vector<DataMessageReceived>& messages) {
  for (const auto& message : messages) {
    RTC_LOG(LS_INFO) << "data message: " << message.data;
  }
}
