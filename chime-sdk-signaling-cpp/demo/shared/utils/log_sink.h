//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef LOG_SINK_H_
#define LOG_SINK_H_

#include "utils/logging.h"

#include "webrtc/rtc_base/logging.h"

using namespace chime;

class LogSink : public rtc::LogSink {
 public:
  virtual void OnLogMessage(const std::string& msg, rtc::LoggingSeverity severity) override {
    switch (severity) {
      case rtc::LS_VERBOSE:
        CHIME_LOG(LogLevel::kVerbose, msg);
        break;
      case rtc::LS_INFO:
        CHIME_LOG(LogLevel::kInfo, msg);
        break;
      case rtc::LS_WARNING:
        CHIME_LOG(LogLevel::kWarning, msg);
        break;
      case rtc::LS_ERROR:
        CHIME_LOG(LogLevel::kError, msg);
        break;
      default:
        CHIME_LOG(LogLevel::kVerbose, msg);
        break;
    }
  }

  virtual void OnLogMessage(const std::string& msg) override {}
};

#endif  // LOG_SINK_H_
