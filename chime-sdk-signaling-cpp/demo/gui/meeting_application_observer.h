// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#ifndef MEETING_APPLICATION_OBSERVER_H
#define MEETING_APPLICATION_OBSERVER_H

#include "utils/logging.h"

#include "controllers/meeting_controller.h"
#include "controllers/meeting_controller_configuration.h"
#include "controllers/meeting_controller_dependencies.h"
#include "observers/audio_events_observer.h"
#include "observers/lifecycle_observer.h"
#include "observers/presence_events_observer.h"
#include "observers/session_description_observer.h"
#include "observers/video_events_observer.h"
#include "observers/data_message_observer.h"
#include "observers/peer_connection_observer.h"

#include "signaling/signaling_client.h"
#include "signaling/signaling_client_configuration.h"
#include "signaling/default_signaling_client_factory.h"

#include "video_conferencing_application.h"

#include <iostream>
#include <memory>


class MeetingApplicationObserver : public VideoConferencingApplicationObserver {
public:
    explicit MeetingApplicationObserver(std::shared_ptr<MeetingController> controller);
    virtual ~MeetingApplicationObserver() {}

    void onStartConference() override;
    void onStopConference() override;
    void onEnableVideo() override;
    void onDisableVideo() override;
    void onMuteAudio() override;
    void onUnmuteAudio() override;

private:
    std::shared_ptr<MeetingController> meeting_controller_;
};

#endif // MEETING_APPLICATION_OBSERVER_H
