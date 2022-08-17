// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SIGNALING_SDK_LOG_LEVEL_H_
#define SIGNALING_SDK_LOG_LEVEL_H_

namespace chime {

enum class LogLevel {
  kVerbose = 0,

  kDebug = 1,

  kInfo = 2,

  kWarning = 3,

  kError = 4,

  kOff = 5
};
}  // namespace chime

#endif  // SIGNALING_SDK_LOG_LEVEL_H_
