//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef PEER_CONNECTION_OBSERVER_H_
#define PEER_CONNECTION_OBSERVER_H_

#include "controllers/meeting_controller.h"

#include "webrtc/api/data_channel_interface.h"
#include "webrtc/api/peer_connection_interface.h"
#include "webrtc/api/scoped_refptr.h"
#include "webrtc/api/jsep.h"
#include "webrtc/api/rtp_transceiver_interface.h"

using namespace chime;

class PeerConnectionObserver : public webrtc::PeerConnectionObserver {
 public:
  PeerConnectionObserver(MeetingController* controller);
  virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}
  virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {}
  virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  virtual void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
  virtual void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

 private:
  MeetingController* controller_ = nullptr;
};

#endif  // PEER_CONNECTION_OBSERVER_H_
