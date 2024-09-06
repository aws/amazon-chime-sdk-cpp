cmake_minimum_required(VERSION 3.17)

project(AmazonChimeSignalingSdkCppCli VERSION 0.1.0
        DESCRIPTION "Cli for Amazon Chime's C++ Signaling SDK"
        LANGUAGES C CXX)

option(ENABLE_ASAN OFF "")
set(CMAKE_CXX_STANDARD 17)
set(CLI_NAME my_cli)
set(WORKSPACE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." CACHE INTERNAL "Path to the workspace containing this project and its internal dependencies")
set(CHIME_SIGNAL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(VANILLA_WEBRTC_SRC "${WORKSPACE_SRC}/webrtc-build" CACHE STRING "Path to webRTC folder")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -g -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

add_executable(${CLI_NAME} 
        main.cc
        meeting_application_observer.cc
        backends/imgui.cc
        backends/imgui.h
        ../shared/controllers/meeting_controller.h
        ../shared/controllers/meeting_controller.cc
        ../shared/controllers/meeting_controller_dependencies.h
        ../shared/controllers/meeting_controller_configuration.h
        ../shared/observers/audio_events_observer.h
        ../shared/observers/audio_events_observer.cc
        ../shared/observers/lifecycle_observer.h
        ../shared/observers/lifecycle_observer.cc
        ../shared/observers/presence_events_observer.h
        ../shared/observers/presence_events_observer.cc
        ../shared/observers/session_description_observer.h
        ../shared/observers/session_description_observer.cc
        ../shared/observers/session_description_observer_adapter.h
        ../shared/observers/session_description_observer_adapter.cc
        ../shared/observers/data_message_observer.h
        ../shared/observers/data_message_observer.cc
        ../shared/observers/peer_connection_observer.h
        ../shared/observers/peer_connection_observer.cc
        ../shared/observers/video_events_observer.h
        ../shared/observers/video_events_observer.cc
        ../shared/video/fake_video_source.cc
        ../shared/video/fake_video_source.h
        ../shared/video/write_to_file_yuv_video_sink.cc
        ../shared/video/write_to_file_yuv_video_sink.h
        ../shared/util/fetch_from_serverless_demo.cc
        ../shared/util/fetch_from_serverless_demo.h
        )

include(FetchContent)

FetchContent_Declare(httplib SYSTEM
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
    GIT_TAG a609330e4c6374f741d3b369269f7848255e1954 # v0.14.1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(httplib)
target_link_libraries(${CLI_NAME} PRIVATE httplib)


FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)
target_link_libraries(${CLI_NAME} PRIVATE nlohmann_json::nlohmann_json)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
FetchContent_MakeAvailable(glfw)

target_link_libraries(${CLI_NAME} PUBLIC glfw)
target_include_directories(glfw PUBLIC ${glfw_external_SOURCE_DIR})

FetchContent_Declare(imgui_external
	URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.1.tar.gz
	EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(imgui_external)

add_library(imgui
	${imgui_external_SOURCE_DIR}/imgui.cpp
	${imgui_external_SOURCE_DIR}/imgui_draw.cpp
	${imgui_external_SOURCE_DIR}/imgui_tables.cpp
	${imgui_external_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_external_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	${imgui_external_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(imgui PUBLIC ${imgui_external_SOURCE_DIR})

find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

target_link_libraries(${CLI_NAME} PUBLIC imgui)


target_include_directories(${CLI_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(${CLI_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../shared/")

# Expose include paths for upstream projects
target_include_directories(
        ${CLI_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_LINUX)
target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_POSIX)
target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_APM_DEBUG_DUMP=0)

find_package(LibSignaling REQUIRED)
target_link_libraries(${CLI_NAME}
        PUBLIC
        LibSignaling::LibSignaling)

find_package(LibWebRTC REQUIRED)
target_link_libraries(${CLI_NAME} PUBLIC LibWebRTCExtras::LibWebRTCExtras)
target_link_libraries(${CLI_NAME} PUBLIC LibWebRTC::LibWebRTC)

find_package(X11 REQUIRED)
if(X11_FOUND)
    include_directories(${X11_INCLUDE_DIR})
    link_libraries(${X11_LIBRARIES})
endif()
target_link_libraries(${CLI_NAME} PUBLIC glfw ${X11_LIBRARIES})

target_link_libraries(${CLI_NAME} PRIVATE -lcap)

if (NOT MSVC)
    target_compile_options(${CLI_NAME} PRIVATE "-fno-rtti")
endif()


find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)

# Include directories
include_directories(${CLI_NAME} PRIVATE ${GLIB_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS})
link_directories(${CLI_NAME} PRIVATE ${GLIB_LIBRARY_DIRS} ${GOBJECT_LIBRARY_DIRS} ${GIO_LIBRARY_DIRS})
# Link libraries to your target
target_link_libraries(${CLI_NAME} PRIVATE  ${GLIB_LIBRARIES} ${GOBJECT_LIBRARIES} ${GIO_LIBRARIES})