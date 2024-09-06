// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "utils/fetch_from_serverless_demo.h"

#include <iostream>
#include <memory>


class MeetingApplicationObserver : public VideoConferencingApplicationObserver {
public:
    explicit MeetingApplicationObserver(MeetingController* controller);
    virtual ~MeetingApplicationObserver() {}

    void onMeetingJoinRequested(const std::string& url, const std::string& meeting_name, const std::string& attendee_name) override;
    void onStartConference() override;
    void onStopConference() override;
    void onEnableVideo() override;
    void onDisableVideo() override;
    void onMuteAudio() override;
    void onUnmuteAudio() override;

private:
    std::shared_ptr<MeetingController> meetingController;
};
