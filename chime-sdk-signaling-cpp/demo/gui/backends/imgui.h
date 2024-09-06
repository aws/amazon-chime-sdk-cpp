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
    std::shared_ptr<VideoConferencingTile> addRemoteVideo() override;

private:
    GLFWwindow* window_;
    VideoConferencingApplicationObserver* observer_;
    bool running_;

    void initialize();
    void renderGui();
    void setupImGui(GLFWwindow* window, const char* glsl_version);
    void cleanupImGui();
};

#endif // IMGUI_VIDEO_CONFERENCING_APPLICATION_H