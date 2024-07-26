// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "video/fake_video_source.h"

#include "rtc_base/timestamp_aligner.h"
#include "rtc_base/time_utils.h"

void FakeVideoSource::Broadcast(const webrtc::VideoFrame& frame) {
  for (auto sink : webrtc_sinks_) {
    sink->OnFrame(frame);
  }
}

void FakeVideoSource::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) {
  auto it = std::find(webrtc_sinks_.begin(), webrtc_sinks_.end(), sink);
  if (it != webrtc_sinks_.end()) {
    webrtc_sinks_.erase(it);
  }
}

void FakeVideoSource::AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                                      const rtc::VideoSinkWants& wants) {
  RemoveSink(sink);
  webrtc_sinks_.push_back(sink);
}

void FakeVideoSource::Start(int width, int height, int fps) {
  if (width < 0 || height < 0 || fps < 0) {
    return;
  }
  const long long int kNsWaitInterval = ceil(1000000000 / fps);
  send_frames_ = true;

  frame_generator_thread_ = std::thread([=]() {
    // Used to alternate frame color
    // approximately once every second.
    int frame_count = 0;
    while (send_frames_) {
      frame_count %= (2 * fps);
      frame_count++;
      // wait time approximately corresponds to given fps
      std::this_thread::sleep_for(std::chrono::nanoseconds(kNsWaitInterval));
      rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Create(width, height);
      if (frame_count > fps) {
        buffer->SetBlack(buffer.get());
      } else {
        buffer->InitializeData();
      }
      auto timestamp_us =
          std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
              .count();
      webrtc::VideoFrame frame =
          webrtc::VideoFrame::Builder().set_video_frame_buffer(buffer).set_timestamp_us(timestamp_us).build();
      Broadcast(frame);
    }
  });
}

void FakeVideoSource::Stop() {
  send_frames_ = false;
  if (frame_generator_thread_.joinable()) {
    frame_generator_thread_.join();
  }
}
