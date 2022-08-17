// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_DATA_MESSAGE_H_
#define CHIME_DATA_MESSAGE_H_

#include <string>

namespace chime {

struct DataMessage {
  // The topic this message was sent to
  std::string topic;
  // Data payload
  std::string data;

  std::string DebugString() const {
    return "{ topic: " + topic + ", data: " + data + " }";
  }
};

}  // namespace chime
#endif  // CHIME_DATA_MESSAGE_H_
