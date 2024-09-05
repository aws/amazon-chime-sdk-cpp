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

#include "backends/imgui.h"
#include "video_conferencing_application.h"

#include <iostream>
#include <memory>

using namespace chime;

class MeetingApplicationObserver : public VideoConferencingApplicationObserver {
private:
    MeetingController* meetingController; // Use a raw pointer or a smart pointer based on ownership needs

public:
    // Constructor accepting a pointer to an existing MeetingController
    explicit MeetingApplicationObserver(MeetingController* controller)
        : meetingController(controller) {}

    // Destructor
    virtual ~MeetingApplicationObserver() {
        // Cleanup if needed, but do not delete if not owning the pointer
    }

    // Implement observer methods to forward Application actions to the MeetingController
    void onStartConference() override {
        if (meetingController) {
            meetingController->Start();
        }
    }

    void onStopConference() override {
        if (meetingController) {
            meetingController->Stop();
        }
    }

    void onEnableVideo() override {
        if (meetingController) {
            meetingController->StartLocalVideo();
        }
    }

    void onDisableVideo() override {
        // Add handling if the API supports stopping the video
    }

    void onMuteAudio() override {
        // Add handling if the API supports muting the audio
    }

    void onUnmuteAudio() override {
        // Add handling if the API supports unmuting the audio
    }
};


int main(int argc, char* argv[]) {
  webrtc::PeerConnectionFactoryDependencies peer_connection_factory_dependencies;

  std::optional<MeetingSessionConfiguration> meeting_configuration = createMeetingConfiguration(result);
  if (!meeting_configuration) {
      std::cout << "Could not create meeting configuration. You may be missing a value" << std::endl;
      return 0;
  }

  SignalingClientConfiguration signaling_configuration;
  signaling_configuration.meeting_configuration = *meeting_configuration;

  DefaultSignalingDependencies signaling_dependencies {};
  auto client =
      DefaultSignalingClientFactory::CreateSignalingClient(signaling_configuration, std::move(signaling_dependencies));

  MeetingControllerConfiguration configuration;
  configuration.meeting_configuration = {};
  auto session_description_observer = std::make_unique<SessionDescriptionObserver>();
  std::shared_ptr<MeetingController> controller
      = MeetingController::Create(configuration, std::move(client), session_description_observer.get());

  session_description_observer->controller_ = controller.get();
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

  controller->Start();

    try {
        MeetingGuiObserver observer(meetingController.get());
        // Set up the GUI with the observer
        ImGuiVideoConferencingGui gui(&observer);
        gui.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
