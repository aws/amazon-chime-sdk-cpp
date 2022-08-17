// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_DATA_MESSAGE_RECEIVED_H_
#define CHIME_DATA_MESSAGE_RECEIVED_H_

#include "utils/attendee.h"
#include "data_message/data_message.h"

namespace chime {

struct DataMessageReceived : DataMessage {
  // Monotonically increasing server ingest time
  int64_t timestamp_ms;
  // Sender attendee data
  Attendee attendee;
};

} // namespace chime
#endif  // CHIME_DATA_MESSAGE_RECEIVED_H_
