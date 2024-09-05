#ifndef IMGUI_VIDEO_CONFERENCING_APPLICATION_H
#define IMGUI_VIDEO_CONFERENCING_APPLICATION_H

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include "video_conferencing_application.h" // Ensure this is the correct path to your base class

class ImGuiVideoConferencingApplication : public VideoConferencingApplication {
public:
    explicit ImGuiVideoConferencingApplication(VideoConferencingApplicationObserver* observer);
    ~ImGuiVideoConferencingApplication();

    void run() override;
    void stop() override;
    std::shared_ptr<RemoteVideoTile> addRemoteVideo() override;

private:
    GLFWwindow* window;
    VideoConferencingApplicationObserver* observer;
    bool running;

    void initialize();
    void renderGui();
    void setupImGui(GLFWwindow* window);
    void cleanupImGui();
};

#endif // IMGUI_VIDEO_CONFERENCING_APPLICATION_H