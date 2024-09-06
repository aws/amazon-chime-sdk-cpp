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
#include "meeting_application_observer.h"

#include <iostream>
#include <memory>

using namespace chime;
#define CXXOPTS_NO_RTTI

#include "cxxopts.hpp"

#include <iostream>
#include <memory>

using namespace chime;

std::optional<MeetingSessionConfiguration> createMeetingConfiguration(const cxxopts::ParseResult& result) {
    auto attendee_id = result["attendee_id"].as<std::string>();
    auto external_user_id = result["external_user_id"].as<std::string>();
    auto join_token = result["join_token"].as<std::string>();
    auto audio_host_url = result["audio_host_url"].as<std::string>();
    auto signaling_url = result["signaling_url"].as<std::string>();
    auto meeting_id = result["meeting_id"].as<std::string>();
    auto external_meeting_id = result["external_meeting_id"].as<std::string>();

    if (attendee_id.empty() || external_user_id.empty() || join_token.empty() ||
        audio_host_url.empty() || signaling_url.empty() || meeting_id.empty() ||
        external_meeting_id.empty()) {
      return std::nullopt;
    }

    MeetingSessionCredentials credentials{attendee_id, external_user_id, join_token};
    MeetingSessionURLs urls{audio_host_url, signaling_url};
    MeetingSessionConfiguration configuration{meeting_id, external_meeting_id, std::move(credentials), std::move(urls)};
    
    return configuration;
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("my_cli", "Running Cli demo application");
  options.allow_unrecognised_options();
  try {
    options.add_options()("l,log_level", "Log level [verbose, debug, info, warning, error, off]",
                          cxxopts::value<std::string>()->default_value("info"))(
        "m,meeting_id", "Meeting ID [JoinInfo.Meeting.MeetingId]", cxxopts::value<std::string>()->default_value(""))(
        "e,external_meeting_id", "External meeting ID [JoinInfo.Meeting.ExternalMeetingId]",
        cxxopts::value<std::string>()->default_value(""))(
        "h,audio_host_url", "Audio host URL [JoinInfo.Meeting.MediaPlacement.AudioHostUrl]",
        cxxopts::value<std::string>()->default_value(""))(
        "s,signaling_url", "Signaling URL [JoinInfo.Meeting.MediaPlacement.SignalingUrl]",
        cxxopts::value<std::string>()->default_value(""))("a,attendee_id",
                                                          "Attendee ID [JoinInfo.Attendee.Attendee.AttendeeId]",
                                                          cxxopts::value<std::string>()->default_value(""))(
        "u,external_user_id", "External attendee ID [JoinInfo.Attendee.Attendee.ExternalUserId]",
        cxxopts::value<std::string>()->default_value(""))("j,join_token",
                                                          "Join token [JoinInfo.Attendee.Attendee.JoinToken]",
                                                          cxxopts::value<std::string>()->default_value(""));
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    return 0;
  }

  auto result = options.parse(argc, argv);
  webrtc::PeerConnectionFactoryDependencies peer_connection_factory_dependencies;

  std::string usr = result["external_user_id"].as<std::string>();
  std::cout << "\nAttendee Name: " << usr.substr(usr.find("#") + 1) << std::endl;
  std::cout << "Attendee ID: " << result["attendee_id"].as<std::string>() << std::endl;
  std::cout << "Meeting Name: " << result["external_meeting_id"].as<std::string>() << std::endl;
  std::cout << "Meeting ID: " << result["meeting_id"].as<std::string>() << std::endl;
  std::cout << "Current Log Level: " << result["log_level"].as<std::string>() << std::endl;

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
  configuration.meeting_configuration = *meeting_configuration;
  configuration.log_level = result["log_level"].as<std::string>();
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

    try {
        MeetingApplicationObserver observer(controller);
        // Set up the GUI with the observer
        ImGuiVideoConferencingApplication gui(&observer);
        gui.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
