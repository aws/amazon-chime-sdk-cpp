//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "logging.h"

#include <ctime>
#include <iostream>

namespace chime {

LogLevel current_level = LogLevel::kInfo;

std::string LogLevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::kVerbose:
      return "VERBOSE";
    case LogLevel::kDebug:
      return "DEBUG";
    case LogLevel::kInfo:
      return "INFO";
    case LogLevel::kWarning:
      return "WARNING";
    case LogLevel::kError:
      return "ERROR";
    case LogLevel::kOff:
      return "OFF";
    default:
      return "INFO";
  }
}

LogLevel StringToLogLevel(std::string level) {
  for (auto& c : level) c = std::toupper(c);
  if (level == "VERBOSE")
    return LogLevel::kVerbose;
  if (level == "DEBUG")
    return LogLevel::kDebug;
  if (level == "INFO")
    return LogLevel::kInfo;
  if (level == "WARNING")
    return LogLevel::kWarning;
  if (level == "ERROR")
    return LogLevel::kError;
  if (level == "OFF")
    return LogLevel::kOff;
  
  return LogLevel::kInfo;
}

void SetSignalingLogLevel(LogLevel level) {
  current_level = level;
}

void SetSignalingLogLevel(std::string level) {
  current_level = StringToLogLevel(level);
}

void Log(LogLevel current_log_level, LogLevel given_log_level, const std::string& log_message) {
  auto current_level_val = static_cast<int>(current_log_level);
  auto given_level_val = static_cast<int>(given_log_level);
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  char buffer[80];
  strftime(buffer,sizeof(buffer),"%H:%M:%S", &tm);
  std::string str(buffer);

  if (current_level_val > given_level_val) return;
  std::cout << "(" << str << ") " << "[" << LogLevelToString(given_log_level) << "]: " << log_message << std::endl;
  }
}  // namespace chime
