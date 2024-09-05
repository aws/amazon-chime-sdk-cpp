// ImGuiVideoConferencingApplication.cc
#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

ImGuiVideoConferencingApplication::ImGuiVideoConferencingApplication(VideoConferencingApplicationObserver* observer)
    : window(nullptr), observer(observer), running(false) {
    initialize();
}

ImGuiVideoConferencingApplication::~ImGuiVideoConferencingApplication() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void ImGuiVideoConferencingApplication::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        throw std::runtime_error("Failed to initialize GLFW");
    }

    window = glfwCreateWindow(1280, 720, "Video Conferencing App", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window\n";
        throw std::runtime_error("Failed to create GLFW window");
    }
}

void ImGuiVideoConferencingApplication::run() {
    setupImGui(window);

    running = true;
    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderGui();
        glfwSwapBuffers(window);
    }

    cleanupImGui();
}

void ImGuiVideoConferencingApplication::stop() {
    running = false;
}

void ImGuiVideoConferencingApplication::renderGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Video Conferencing Controls");
    if (ImGui::Button("Start Conference")) observer->onStartConference();
    if (ImGui::Button("Stop Conference")) observer->onStopConference();
    if (ImGui::Button("Enable Video")) observer->onEnableVideo();
    if (ImGui::Button("Disable Video")) observer->onDisableVideo();
    if (ImGui::Button("Mute Audio")) observer->onMuteAudio();
    if (ImGui::Button("Unmute Audio")) observer->onUnmuteAudio();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_COLOR_BUFFER_BIT);
}

std::shared_ptr<VideoConferencingTile> ImGuiVideoConferencingApplication::addRemoteVideo() {
    // Implementation to add a remote video tile
    return nullptr;
}

void ImGuiVideoConferencingApplication::setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
}

void ImGuiVideoConferencingApplication::cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
