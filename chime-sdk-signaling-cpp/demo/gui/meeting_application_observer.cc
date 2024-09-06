#include "meeting_application_observer.h"

// Constructor accepting a pointer to an existing MeetingController
MeetingApplicationObserver::MeetingApplicationObserver(MeetingController* controller)
    : meeting_controller_(controller) {}

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