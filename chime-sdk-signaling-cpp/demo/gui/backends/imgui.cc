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

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


void ImGuiVideoConferencingApplication::initialize() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;

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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    setupImGui(window, glsl_version);
}

void ImGuiVideoConferencingApplication::setupImGui(GLFWwindow* window, const char* glsl_version) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void ImGuiVideoConferencingApplication::run() {
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

void ImGuiVideoConferencingApplication::cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
