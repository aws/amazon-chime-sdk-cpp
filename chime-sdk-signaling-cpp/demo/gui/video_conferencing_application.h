// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#include <cstdint>
#include <memory>

class VideoConferencingApplicationObserver {
public:
    virtual ~VideoConferencingApplicationObserver() {}

    virtual void onStartConference() = 0;
    virtual void onStopConference() = 0;
    virtual void onEnableVideo() = 0;
    virtual void onDisableVideo() = 0;
    virtual void onMuteAudio() = 0;
    virtual void onUnmuteAudio() = 0;
};

class VideoConferencingTile {
public:
    virtual ~VideoConferencingTile() {}
    virtual void renderFrame(const uint8_t* frameData, int width, int height) = 0;
};

class VideoConferencingApplication {
public:
    virtual ~VideoConferencingApplication() {}
    virtual void run() = 0;   // Start the GUI event loop
    virtual void stop() = 0;  // Signal to stop the GUI event loop
    virtual std::shared_ptr<VideoConferencingTile> addRemoteVideo() = 0; // Add and manage a remote video tile
};