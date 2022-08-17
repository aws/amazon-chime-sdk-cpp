//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SESSION_DESCRIPTION_OBSERVER_H_
#define SESSION_DESCRIPTION_OBSERVER_H_

#include "observers/session_description_observer_adapter.h"

#include "webrtc/api/jsep.h"
#include "webrtc/api/rtc_error.h"

// Forward declaration
class MeetingController;

class SessionDescriptionObserver : public CreateSessionDescriptionObserverAdapter,
                                   public SetSessionDescriptionObserverAdapter {
 public:
  virtual void OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) override;
  virtual void OnCreateSessionDescriptionFailure(const webrtc::RTCError& error) override;
  virtual void OnSetSessionDescriptionSuccess(bool is_local) override;
  virtual void OnSetSessionDescriptionFailure(bool is_local, const webrtc::RTCError& error) override;

  MeetingController* controller_ = nullptr;
};

#endif  // SESSION_DESCRIPTION_OBSERVER_H_
