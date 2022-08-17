
#ifndef CHIME_SIGNALING_LOGGING_H_
#define CHIME_SIGNALING_LOGGING_H_

#include "utils/logger/log_level.h"

#include <unordered_map>
#include <string>

namespace chime {

extern LogLevel current_level;

// TODO: improve logging
std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(std::string level);
void Log(LogLevel current_log_level, LogLevel given_log_level, const std::string& log_message);
void SetSignalingLogLevel(LogLevel level);
void SetSignalingLogLevel(std::string level);

#define CHIME_LOG(log_level, log_message) \
  (Log(current_level, log_level, log_message));
}  // namespace chime

#endif  // CHIME_SIGNALING_LOGGING_H_
