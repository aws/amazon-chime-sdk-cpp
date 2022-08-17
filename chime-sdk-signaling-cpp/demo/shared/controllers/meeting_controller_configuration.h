// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CONTROLLER_CONFIGURATION_H_
#define CONTROLLER_CONFIGURATION_H_

#include "session/meeting_session_configuration.h"

#include <string>

using namespace chime;

/**
 * Defines configuration needed to create the controller
 */
struct MeetingControllerConfiguration {
  MeetingSessionConfiguration meeting_configuration;

  // Audio files are 48kHz raw stereo pcm.
  std::string input_audio_filename = "";
  std::string output_audio_filename = "media_out/remote_audio_recording.pcm";
  std::string log_level = "info";
};

#endif  // CONTROLLER_CONFIGURATION_H_
