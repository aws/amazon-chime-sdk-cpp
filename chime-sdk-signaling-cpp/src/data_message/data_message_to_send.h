// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_DATA_MESSAGE_TO_SEND_H_
#define CHIME_DATA_MESSAGE_TO_SEND_H_

#include "data_message/data_message.h"

namespace chime {

struct DataMessageToSend : DataMessage {
  // The milliseconds of lifetime that is available to late subscribers
  int lifetime_ms;
};

} // namespace chime
#endif  // CHIME_DATA_MESSAGE_TO_SEND_H_
