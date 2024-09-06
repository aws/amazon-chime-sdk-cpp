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

std::optional<MeetingSessionConfiguration> fetchCredentialsFromServerlessDemo(const std::string& base_url, const std::string& meeting, const std::string& attendee, const std::string& region);