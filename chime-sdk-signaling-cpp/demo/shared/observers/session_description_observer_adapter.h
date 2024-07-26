//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SESSION_DESCRIPTION_OBSERVER_ADAPTER_H_
#define SESSION_DESCRIPTION_OBSERVER_ADAPTER_H_

#include "webrtc/api/rtc_event_log/rtc_event_log.h"
#include "webrtc/api/jsep.h"

// These adapters address the overlapping callback signatures of
//   webrtc::CreateSessionDescriptionObserver and
//   webrtc::SetSessionDescriptionObserver, allowing a class to receive
//   callbacks from both.
class CreateSessionDescriptionObserverAdapter {
 public:
  virtual void OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) = 0;
  virtual void OnCreateSessionDescriptionFailure(const webrtc::RTCError& error) = 0;
};

class SetSessionDescriptionObserverAdapter {
 public:
  virtual void OnSetSessionDescriptionSuccess(bool is_local) = 0;
  virtual void OnSetSessionDescriptionFailure(bool is_local, const webrtc::RTCError& error) = 0;
};

class CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver {
 public:
  CreateSessionDescriptionObserver(CreateSessionDescriptionObserverAdapter* adapter);
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
  virtual void OnFailure(webrtc::RTCError error) override;

 private:
  CreateSessionDescriptionObserverAdapter* adapter_;
};
class SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
 public:
  SetSessionDescriptionObserver(bool is_local, SetSessionDescriptionObserverAdapter* adapter);
  virtual void OnSuccess() override;
  virtual void OnFailure(webrtc::RTCError error) override;

 private:
  bool is_local_;
  SetSessionDescriptionObserverAdapter* adapter_ = nullptr;
};

#endif  // SESSION_DESCRIPTION_OBSERVER_ADAPTER_H_
