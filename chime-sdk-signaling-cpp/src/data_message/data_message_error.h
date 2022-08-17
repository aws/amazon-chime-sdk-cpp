// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.


#ifndef CHIME_DATA_MESSAGE_ERROR_H_
#define CHIME_DATA_MESSAGE_ERROR_H_

#include "data_message/data_message.h"

namespace chime {

// Reason to describe why data message sent was failed.
enum class DataMessageSendErrorReason {
  // Server throttled or rejected message
  kThrottled = 0,

  // Given input was invalid
  kInvalidTopic = 1,
  kInvalidDataMessageSize = 2,
  kInvalidLifeTimeMs = 3,
};

// Error that occur when failed to send the data message
struct DataMessageSendError {
  DataMessage data_message;
  DataMessageSendErrorReason reason;
  
  static std::string ReasonToString(DataMessageSendErrorReason reason) {
    switch (reason) {
      case DataMessageSendErrorReason::kThrottled :
        return "Throttled";
      case DataMessageSendErrorReason::kInvalidTopic :
        return "InvalidTopic";
      case DataMessageSendErrorReason::kInvalidDataMessageSize :
        return "InvalidDataMessageSize";
      case DataMessageSendErrorReason::kInvalidLifeTimeMs :
        return "InvalidLifeTimeMs";
    }
  }

  std::string DebugString() const {
    return "{ data_message: " + data_message.DebugString() + ", reason: " + ReasonToString(reason) + " }";
  } 
};

}
#endif  // CHIME_DATA_MESSAGE_ERROR_H_
