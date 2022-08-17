// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "write_to_file_yuv_video_sink.h"

#include "webrtc/rtc_base/logging.h"

#include <iostream>
#include <string>

// Opens the YUV file and names it by the video id. For example: videoId_5.yuv
WriteToFileYuvVideoSink::WriteToFileYuvVideoSink(const std::string& video_id) {
  auto file_name = std::string("videoId_") + video_id + ".yuv";
  file_ = fopen(file_name.c_str(), "wb+");
}

WriteToFileYuvVideoSink::~WriteToFileYuvVideoSink() { fclose(file_); }

// Gets I420 video frame buffer and writes to
// the file.
void WriteToFileYuvVideoSink::OnFrame(const webrtc::VideoFrame& frame) {
  int width = frame.width();
  int height = frame.height();

  rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(frame.video_frame_buffer()->ToI420());
  int stride_uv = buffer->StrideU();
  int chroma_size = stride_uv * height / 2;
  fwrite(buffer->DataY(), 1, buffer->StrideY() * height, file_);
  fwrite(buffer->DataU(), 1, chroma_size, file_);
  fwrite(buffer->DataV(), 1, chroma_size, file_);
  fflush(file_);
  RTC_LOG(LS_VERBOSE) << "Saving frame of size: " << buffer->StrideY() << "X" << height;
}
