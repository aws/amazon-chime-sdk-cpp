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

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <optional>
#include <string>

std::optional<MeetingSessionConfiguration> fetchCredentialsFromServerlessDemo(const std::string& base_url, const std::string& meeting, const std::string& attendee, const std::string& region) {
    // Create the request URL
    std::string request_url = base_url + "join?title=" + meeting + "&name=" + attendee + "&region=" + region;

    // Set up HTTP client
    httplib::Client cli(base_url.c_str());

    // Make the POST request
    auto res = cli.Post(request_url.c_str());
    if (!res || res->status != 200) {
        std::cerr << "Failed to make request or server returned an error: " << res->status << std::endl;
        return std::nullopt;
    }

    // Parse JSON data from response
    auto json = nlohmann::json::parse(res->body);
    std::string attendee_id = json["JoinInfo"]["Attendee"]["Attendee"]["AttendeeId"];
    std::string audio_host_url = json["JoinInfo"]["Meeting"]["Meeting"]["MediaPlacement"]["AudioHostUrl"];
    std::string external_meeting_id = json["JoinInfo"]["Meeting"]["Meeting"]["ExternalMeetingId"];
    std::string external_user_id = json["JoinInfo"]["Attendee"]["Attendee"]["ExternalUserId"];
    std::string join_token = json["JoinInfo"]["Attendee"]["Attendee"]["JoinToken"];
    std::string meeting_id = json["JoinInfo"]["Meeting"]["Meeting"]["MeetingId"];
    std::string signaling_url = json["JoinInfo"]["Meeting"]["Meeting"]["MediaPlacement"]["SignalingUrl"];

    // Validate required fields
    if (attendee_id.empty() || external_user_id.empty() || join_token.empty() ||
        audio_host_url.empty() || signaling_url.empty() || meeting_id.empty() ||
        external_meeting_id.empty()) {
        std::cerr << "One or more required fields are empty." << std::endl;
        return std::nullopt;
    }

    MeetingSessionCredentials credentials{attendee_id, external_user_id, join_token};
    MeetingSessionURLs urls{audio_host_url, signaling_url};
    MeetingSessionConfiguration configuration{meeting_id, external_meeting_id, std::move(credentials), std::move(urls)};

    return configuration;
}