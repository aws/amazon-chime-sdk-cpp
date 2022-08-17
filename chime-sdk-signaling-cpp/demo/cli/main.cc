// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#include "utils/logging.h"

#include "controllers/meeting_controller.h"
#include "controllers/meeting_controller_configuration.h"
#include "controllers/meeting_controller_dependencies.h"
#include "controllers/keypress_controller.h"
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

#define CXXOPTS_NO_RTTI

#include "cxxopts.hpp"

#include <iostream>
#include <memory>

using namespace chime;

MeetingSessionConfiguration createMeetingConfiguration(const cxxopts::ParseResult& result) {
  try {
    MeetingSessionCredentials credentials{result["attendee_id"].as<std::string>(),
                                          result["external_user_id"].as<std::string>(),
                                          result["join_token"].as<std::string>()};

    MeetingSessionURLs urls{result["audio_host_url"].as<std::string>(), result["signaling_url"].as<std::string>()};

    MeetingSessionConfiguration configuration{result["meeting_id"].as<std::string>(),
                                              result["external_meeting_id"].as<std::string>(), std::move(credentials),
                                              std::move(urls)};
    return configuration;
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  MeetingSessionConfiguration c{};
  return c;
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
        "s,signaling_url", "Singaling URL [JoinInfo.Meeting.MediaPlacement.SignalingUrl]",
        cxxopts::value<std::string>()->default_value(""))("a,attendee_id",
                                                          "Attendee ID [JoinInfo.Attendee.Attendee.AttendeeId]",
                                                          cxxopts::value<std::string>()->default_value(""))(
        "u,external_user_id", "External attendee ID [JoinInfo.Attendee.Attendee.ExternalUserId]",
        cxxopts::value<std::string>()->default_value(""))("j,join_token",
                                                          "Join token [JoinInfo.Attendee.Attendee.JoinToken]",
                                                          cxxopts::value<std::string>()->default_value(""))(
        "f,send_audio_file_name", "Audio file to play 16KHz, 16 bit PCM wave file only",
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
  std::cout << "Audio Filename: " << result["send_audio_file_name"].as<std::string>() << std::endl;
  std::cout << "Current Log Level: " << result["log_level"].as<std::string>() << std::endl;

  SignalingClientConfiguration signaling_configuration;
  MeetingSessionConfiguration meeting_configuration = createMeetingConfiguration(result);
  signaling_configuration.meeting_configuration = meeting_configuration;

  DefaultSignalingDependencies signaling_dependencies {};
  auto client =
      DefaultSignalingClientFactory::CreateSignalingClient(signaling_configuration, std::move(signaling_dependencies));

  MeetingControllerConfiguration configuration;
  configuration.meeting_configuration = meeting_configuration;
  configuration.input_audio_filename = result["send_audio_file_name"].as<std::string>();
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

  controller->Start();

  auto keypressController = std::make_unique<KeypressController>(controller);
  return keypressController->Exec();
}
