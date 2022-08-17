//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "session_description_observer_adapter.h"

CreateSessionDescriptionObserver::CreateSessionDescriptionObserver(CreateSessionDescriptionObserverAdapter* adapter)
    : adapter_(adapter) {}

void CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  adapter_->OnCreateSessionDescriptionSuccess(desc);
}

void CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  adapter_->OnCreateSessionDescriptionFailure(error);
}

SetSessionDescriptionObserver::SetSessionDescriptionObserver(bool is_local,
                                                             SetSessionDescriptionObserverAdapter* adapter)
    : is_local_(is_local), adapter_(adapter) {}

void SetSessionDescriptionObserver::OnSuccess() { adapter_->OnSetSessionDescriptionSuccess(is_local_); }

void SetSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  adapter_->OnSetSessionDescriptionFailure(is_local_, error);
}
