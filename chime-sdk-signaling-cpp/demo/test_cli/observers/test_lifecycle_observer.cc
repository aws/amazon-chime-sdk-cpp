//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "test_lifecycle_observer.h"

#include "signaling/signaling_client_start_info.h"
#include "signaling/signaling_client_status.h"

#include "webrtc/rtc_base/logging.h"
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/pc/session_description.h"
#include "webrtc/api/audio_options.h"
#include "webrtc/api/rtp_transceiver_interface.h"
#include "webrtc/rtc_base/ref_counted_object.h"
#include "webrtc/pc/video_track_source.h"
#include "webrtc/api/media_stream_interface.h"
#include "webrtc/api/jsep.h"

#include "utils/cloud_watch_utils.h"

#include <string>
#include <memory>

using namespace chime;

TestLifecycleObserver::TestLifecycleObserver(MeetingController* controller, std::shared_ptr<TestMarker> marker,
                                             PeerConnectionObserver* peer_connection_observer,
                                             VideoEventsObserver* video_events_observer,
                                             SessionDescriptionObserver* session_description_observer)
    : LifecycleObserver(controller, peer_connection_observer, video_events_observer, session_description_observer),
      marker_(std::move(marker)) {}

void TestLifecycleObserver::OnSignalingClientStarted(const SignalingClientStartInfo& join_info) {
  marker_->MarkSuccessful(TestCase::kConnection);
  LifecycleObserver::OnSignalingClientStarted(join_info);
}

void TestLifecycleObserver::OnSignalingClientStopped(const SignalingClientStatus& status) {
  if (status.type != SignalingClientStatusType::kOk) {
    marker_->MarkFailed(TestCase::kConnection);
  }
  LifecycleObserver::OnSignalingClientStopped(status);
}

void TestLifecycleObserver::OnRemoteDescriptionReceived(const std::string& sdp_answer) {
  marker_->MarkSuccessful(TestCase::kSubscription);
  LifecycleObserver::OnRemoteDescriptionReceived(sdp_answer);
}
