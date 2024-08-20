// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#include "utils/logging.h"

#include "controllers/meeting_controller.h"
#include "observers/audio_events_observer.h"
#include "observers/test_lifecycle_observer.h"
#include "observers/presence_events_observer.h"
#include "observers/session_description_observer.h"
#include "observers/video_events_observer.h"
#include "observers/data_message_observer.h"
#include "observers/peer_connection_observer.h"

#include "signaling/signaling_client.h"
#include "signaling/signaling_client_configuration.h"
#include "signaling/default_signaling_client_factory.h"

#include <aws/core/Aws.h>

#include "utils/cloud_watch_utils.h"
#include "utils/test_marker.h"

#define CXXOPTS_NO_RTTI

#include "cxxopts.hpp"

#include <iostream>
#include <string>
#include <memory>
#include <chrono>

using namespace chime;

constexpr const char* CANARY_NAMESPACE = "AmazonChimeSignalingCanary";

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
  cxxopts::Options options("test_cli", "Running Cli demo application");
  options.allow_unrecognised_options();
  Aws::SDKOptions aws_options;
  Aws::InitAPI(aws_options);

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
        "c,is_canary", "Running canary test",
        cxxopts::value<bool>()->default_value("false"))(
        "d,duration", "Duration for tests to run and finish in seconds",
        cxxopts::value<uint32_t>()->default_value("25"))(
        "test_connection", "Running connection test",
        cxxopts::value<bool>()->default_value("false"))(
        "test_subscription", "Running subscription test",
        cxxopts::value<bool>()->default_value("false"));
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
  std::cout << "Is Canary: " << result["is_canary"].as<bool>() << std::endl;
  
  SignalingClientConfiguration signaling_configuration;
  MeetingSessionConfiguration meeting_configuration = createMeetingConfiguration(result);
  signaling_configuration.meeting_configuration = meeting_configuration;

  DefaultSignalingDependencies signaling_dependencies {};
  auto client =
      DefaultSignalingClientFactory::CreateSignalingClient(signaling_configuration, std::move(signaling_dependencies));

  MeetingControllerConfiguration configuration;
  configuration.meeting_configuration = meeting_configuration;
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
  auto test_marker = std::make_shared<TestMarker>();
  auto lifecycle_observer = std::make_unique<TestLifecycleObserver>(controller.get(),
                                                                test_marker,
                                                                peer_connection_observer.get(),
                                                                video_events_observer.get(),
                                                                session_description_observer.get());
  controller->signaling_client_->AddSignalingClientObserver(lifecycle_observer.get());
  auto presence_events_observer = std::make_unique<PresenceEventsObserver>();
  controller->signaling_client_->AddSignalingClientObserver(presence_events_observer.get());

  controller->Start();
  std::chrono::seconds duration(result["duration"].as<uint32_t>());
  std::this_thread::sleep_for(duration);
  controller->Stop();
  std::cout << "Test Connection" << " " << test_marker->IsSuccess(TestCase::kConnection) << std::endl;
  if (result["test_connection"].as<bool>() && !test_marker->IsSuccess(TestCase::kConnection)) {
    std::cout << "Test connection Failed" << std::endl;
    return 1;
  }

  if (result["is_canary"].as<bool>()) {
    if (test_marker->IsSuccess(TestCase::kConnection)) {
      CloudWatchUtils::SendMetricsSuccessFail(CANARY_NAMESPACE, "ConnectionCheck", true);
    } else {
      CloudWatchUtils::SendMetricsSuccessFail(CANARY_NAMESPACE, "ConnectionCheck", false);
    }

    if (test_marker->IsSuccess(TestCase::kSubscription)) {
      CloudWatchUtils::SendMetricsSuccessFail(CANARY_NAMESPACE, "SubscriptionCheck", true);
    } else {
      CloudWatchUtils::SendMetricsSuccessFail(CANARY_NAMESPACE, "SubscriptionCheck", false);
    }
  }

  std::cout << "Test Subscription" << " " << test_marker->IsSuccess(TestCase::kSubscription) << std::endl;

  if (result["test_subscription"].as<bool>() && !test_marker->IsSuccess(TestCase::kSubscription)) {
    std::cout << "Test Subscription Failed" << std::endl;
    return 1;
  }
  Aws::ShutdownAPI(aws_options);

  // Not testing anything so it will be success all the time
  return 0;
}
