//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef MEETING_CONTROLLER_H_
#define MEETING_CONTROLLER_H_

#include "signaling/signaling_client.h"
#include "audio_video/remote_video_source_info.h"

#include "controllers/meeting_controller_configuration.h"
#include "controllers/meeting_controller_dependencies.h"
#include "utils/log_sink.h"
#include "video/fake_video_source.h"

#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/api/media_stream_interface.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/api/video/video_sink_interface.h"
#include "webrtc/api/video/video_frame.h"

#include <memory>
#include <string>
#include <map>

using namespace chime;

class MeetingController {
 public:
  static std::unique_ptr<MeetingController> Create(MeetingControllerConfiguration configuration,
                                                   std::unique_ptr<SignalingClient> signaling_client,
                                                   SessionDescriptionObserver* session_description_observer);
  MeetingController(MeetingControllerConfiguration configuration, MeetingControllerDependencies dependencies);
  ~MeetingController();

  void Start();
  void Stop();

  void StartLocalVideo();
  void SendDataMessage(const std::string& msg);

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
  std::map<std::string, rtc::scoped_refptr<webrtc::RtpTransceiverInterface>> attendee_id_to_remote_video_transceivers_;
  std::map<std::string, std::unique_ptr<rtc::VideoSinkInterface<webrtc::VideoFrame>>> mid_to_remote_video_sinks_;
  std::map<std::string, RemoteVideoSourceInfo> attendee_id_to_remote_video_sources_;
  bool updates_in_flight_ = false;
  std::unique_ptr<SignalingClient> signaling_client_;
  rtc::scoped_refptr<FakeVideoSource> local_video_source_;

  bool RemoteVideoChanged();
  void SendUpdates();
  void SetExternalVideoSource(rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> track,
                              webrtc::VideoTrackInterface::ContentHint content_hint);
  std::map<std::string, RemoteVideoSourceInfo> video_sources_to_subscribe_;

 private:
  // Helpers
  bool IsValidTransceiver(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> in);
  int TURNCandidateCount();
  void UpdateLocalSender();

  std::unique_ptr<rtc::Thread> signaling_thread_;
  std::unique_ptr<LogSink> log_sink_;
  const std::string kDataMessageTopic = "chat";
  const int kDataMessageLifetimeMs = 300000;
  const std::string kVideoCameraLabel = "camera-video";
  // Maxmium kpbs that webrtc will be sending the video
  static constexpr uint32_t kMaxkbps = 800;
  // Minimum kpbs that webrtc will be sending the video
  static constexpr uint32_t kMinkbps = 200;

  // Width and height of fake video source
  static constexpr uint32_t kVideoSendWidth = 480;
  static constexpr uint32_t kVideoSendHeight = 240;

  rtc::scoped_refptr<webrtc::RtpTransceiverInterface> local_video_transceiver_;
  SessionDescriptionObserver* session_description_observer_ = nullptr;
};

#endif  // MEETING_CONTROLLER_H_
