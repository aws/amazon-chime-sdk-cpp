#include "meeting_application_observer.h"

#include "utils/fetch_from_serverless_demo.h"

// Constructor accepting a pointer to an existing MeetingController
MeetingApplicationObserver::MeetingApplicationObserver(MeetingController* controller)
    : meeting_controller_(controller) {}

void MeetingApplicationObserver::onMeetingJoinRequested(const std::string& url, const std::string& meeting_name, const std::string& attendee_name) {
    auto config = fetchCredentialsFromServerlessDemo(url, meeting_name, attendee_name, "us-east-1");
    if (config) {
        std::cout << "Configuration fetched successfully!" << std::endl;
    } else {
        std::cout << "Failed to fetch configuration." << std::endl;
    }

        webrtc::PeerConnectionFactoryDependencies peer_connection_factory_dependencies;

    SignalingClientConfiguration signaling_configuration;
    signaling_configuration.meeting_configuration = {};

    DefaultSignalingDependencies signaling_dependencies {};
    auto client =
        DefaultSignalingClientFactory::CreateSignalingClient(signaling_configuration, std::move(signaling_dependencies));

    MeetingControllerConfiguration configuration;
    configuration.meeting_configuration = {};
    auto session_description_observer = std::make_unique<SessionDescriptionObserver>();
    meeting_controller_ = MeetingController::Create(configuration, std::move(client), session_description_observer.get());

    session_description_observer->controller_ = meeting_controller_.get();
    auto peer_connection_observer = std::make_unique<PeerConnectionObserver>(controller.get());
    auto audio_events_observer = std::make_unique<AudioEventsObserver>();
    controller->signaling_client_->AddSignalingClientObserver(audio_events_observer.get());
    auto data_message_observer = std::make_unique<DataMessageObserver>();
    controller->signaling_client_->AddSignalingClientObserver(data_message_observer.get());
    auto video_events_observer = std::make_unique<VideoEventsObserver>(controller.get(),
                                                                        session_description_observer.get());
    controller->signaling_client_->AddSignalingClientObserver(video_events_observer.get());
    auto lifecycle_observer = std::make_unique<LifecycleObserver>(controller.get(),
                                                                    peer_connection_observer.get(),
                                                                    video_events_observer.get(),
                                                                    session_description_observer.get());
    controller->signaling_client_->AddSignalingClientObserver(lifecycle_observer.get());
    auto presence_events_observer = std::make_unique<PresenceEventsObserver>();
    controller->signaling_client_->AddSignalingClientObserver(presence_events_observer.get());
}

// Implement observer methods to forward Application actions to the MeetingController
void MeetingApplicationObserver::onStartConference() {
    if (meeting_controller_) {
        meeting_controller_->Start();
    }
}

void MeetingApplicationObserver::onStopConference() {
    if (meeting_controller_) {
        meeting_controller_->Stop();
    }
}

void MeetingApplicationObserver::onEnableVideo() {
    if (meeting_controller_) {
        meeting_controller_->StartLocalVideo();
    }
}

void MeetingApplicationObserver::onDisableVideo() {
    // Add handling if the API supports stopping the video
}

void MeetingApplicationObserver::onMuteAudio() {
    // Add handling if the API supports muting the audio
}

void MeetingApplicationObserver::onUnmuteAudio() {
    // Add handling if the API supports unmuting the audio
}