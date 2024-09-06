#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>

ImGuiVideoConferencingApplication::ImGuiVideoConferencingApplication(VideoConferencingApplicationObserver* observer)
    : window_(nullptr), observer_(observer), running_(false) {
    initialize();
}

ImGuiVideoConferencingApplication::~ImGuiVideoConferencingApplication() {
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

static void glfw_error_callback(int error, const char* description)
{
    std::cout << "GLFW Error " << error << " description " << description << std::endl;
}


void ImGuiVideoConferencingApplication::initialize() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cout << "glfwInit" << std::endl;
        return;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    window_ = glfwCreateWindow(360, 240, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window_ == nullptr) {
        std::cout << "glfwCreateWindow" << std::endl;
        return;
    }
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

    setupImGui(window_, glsl_version);
}

void ImGuiVideoConferencingApplication::setupImGui(GLFWwindow* window, const char* glsl_version) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}


void ImGuiVideoConferencingApplication::stop() {
    running_ = false;
}


void ImGuiVideoConferencingApplication::run() {
    running_ = true;
    while (running_ && !glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        renderGui();
        glfwSwapBuffers(window_);
    }

    cleanupImGui();
}

void ImGuiVideoConferencingApplication::renderGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Use the full display area for ImGui window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    
    ImGui::Begin("Video Conferencing Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Start Conference")) observer_->onStartConference();
    if (ImGui::Button("Stop Conference")) observer_->onStopConference();
    if (ImGui::Button("Enable Video")) observer_->onEnableVideo();
    if (ImGui::Button("Disable Video")) observer_->onDisableVideo();
    if (ImGui::Button("Mute Audio")) observer_->onMuteAudio();
    if (ImGui::Button("Unmute Audio")) observer_->onUnmuteAudio();
    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::shared_ptr<VideoConferencingTile> ImGuiVideoConferencingApplication::addRemoteVideo() {
    // Implementation to add a remote video tile
    return nullptr;
}

void ImGuiVideoConferencingApplication::cleanupImGui() {
    std::cout << "cleanupImGui" << std::endl;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
