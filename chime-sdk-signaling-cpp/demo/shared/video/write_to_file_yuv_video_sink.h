// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_WRITE_TO_FILE_YUV_VIDEO_SINK_H_
#define CHIME_WRITE_TO_FILE_YUV_VIDEO_SINK_H_

#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/api/video/video_frame.h"

#include <memory>

// Writes YUV frames to a file. Can be bound to a remote attendee's video
class WriteToFileYuvVideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  // Opens the YUV file
  //
  // - Parameter video_id: Id used to match file names to video streams.
  explicit WriteToFileYuvVideoSink(const std::string& video_id);

  // Closes the YUV file
  ~WriteToFileYuvVideoSink() override;

  // Writes YUV frames to the file
  //
  // - Parameter frame: Frame to be written to file.
  void OnFrame(const webrtc::VideoFrame& frame) override;

 private:
  FILE* file_ = nullptr;
};

#endif  // CHIME_WRITE_TO_FILE_YUV_VIDEO_SINK_H_
