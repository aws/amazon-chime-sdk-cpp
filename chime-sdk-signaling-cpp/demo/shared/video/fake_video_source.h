// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_FAKE_VIDEO_SOURCE_H_
#define CHIME_FAKE_VIDEO_SOURCE_H_

#include "api/media_stream_interface.h"
#include "api/video/i420_buffer.h"
#include "api/video/recordable_encoded_frame.h"
#include "api/video/video_frame.h"
#include "api/video/video_rotation.h"

#include <memory>
#include <thread>
#include <string>
#include <vector>

// Sends video frames from builder to remote attendees.
// This will send black and green frame
class FakeVideoSource : public webrtc::VideoTrackSourceInterface {
 public:
  FakeVideoSource() = default;

  ~FakeVideoSource() override = default;

  void Start(int width = 1280, int height = 720, int fps = 15);
  void Stop();

 private:
  std::vector<rtc::VideoSinkInterface<webrtc::VideoFrame>*> webrtc_sinks_;
  std::thread frame_generator_thread_;
  bool send_frames_ = false;

  void Broadcast(const webrtc::VideoFrame& frame);

  // webrtc::VideoTrackSourceInterface implementation
  void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, const rtc::VideoSinkWants& wants) override;

  void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

  SourceState state() const override { return kLive; }
  bool remote() const override { return false; }
  bool is_screencast() const override { return false; }
  absl::optional<bool> needs_denoising() const override { return absl::nullopt; }

  void RegisterObserver(webrtc::ObserverInterface* observer) override {}
  void UnregisterObserver(webrtc::ObserverInterface* observer) override {}
  bool GetStats(Stats* stats) override { return false; }
  bool SupportsEncodedOutput() const override { return false; }
  void GenerateKeyFrame() override {}
  void AddEncodedSink(rtc::VideoSinkInterface<webrtc::RecordableEncodedFrame>* sink) override {}
  void RemoveEncodedSink(rtc::VideoSinkInterface<webrtc::RecordableEncodedFrame>* sink) override {}
};

#endif  // CHIME_FAKE_VIDEO_SOURCE_H_
