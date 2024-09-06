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
#include "utils/fetch_from_serverless_demo.h"

#include <iostream>
#include <memory>

using namespace chime;

int main(int argc, char* argv[]) {
    try {
        MeetingApplicationObserver observer(nullptr);
        // Set up the GUI with the observer
        ImGuiVideoConferencingApplication gui(&observer);
        gui.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
