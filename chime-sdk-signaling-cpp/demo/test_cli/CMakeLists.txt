cmake_minimum_required(VERSION 3.17...3.20)

project(AmazonChimeSignalingSdkCppCli VERSION 0.1.0
        DESCRIPTION "Cli for Amazon Chime's C++ Signaling SDK"
        LANGUAGES C CXX)

option(ENABLE_ASAN "" OFF)
set(CMAKE_CXX_STANDARD 17)
set(CLI_NAME test_cli)
set(WORKSPACE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." CACHE INTERNAL "Path to the workspace containing this project and its internal dependencies")
set(CHIME_SIGNAL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../")

set(VANILLA_WEBRTC_SRC "${WORKSPACE_SRC}/webrtc-build" CACHE STRING "Webrtc location")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../shared/cmake/")

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
        observers/test_lifecycle_observer.cc
        observers/test_lifecycle_observer.h
        utils/cloud_watch_utils.cc
        utils/test_marker.cc
        ../shared/controllers/meeting_controller.cc
        ../shared/controllers/meeting_controller.h
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
        )

include(FetchContent)
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG        v3.2.1
)

set(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "Enable CXXOPS TEST" FORCE)

FetchContent_MakeAvailable(cxxopts)


FetchContent_Declare(
    aws-sdk-cpp
    GIT_REPOSITORY https://github.com/aws/aws-sdk-cpp.git
    GIT_TAG        49bca4c42af5927cc6a7f49a0633375b9d000928
)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(ENABLE_TESTING OFF)
    set(AUTORUN_UNIT_TESTS OFF)
    set(BUILD_ONLY "monitoring")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ENABLE_TESTING OFF CACHE BOOL "Enable AWS SDK TEST" FORCE)
    set(AUTORUN_UNIT_TESTS OFF CACHE BOOL "Enable AWS SDK UNIT TEST" FORCE)
    set(BUILD_ONLY "monitoring" CACHE STRING "Build only library" FORCE)
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

FetchContent_MakeAvailable(aws-sdk-cpp)

target_include_directories(${CLI_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(${CLI_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../shared")

# Expose include paths for upstream projects
target_include_directories(
        ${CLI_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_MAC)
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_POSIX)
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_APM_DEBUG_DUMP=0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_LINUX)
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_POSIX)
    target_compile_definitions(${CLI_NAME} PUBLIC WEBRTC_APM_DEBUG_DUMP=0)
endif()

find_package(LibSignaling REQUIRED)
target_link_libraries(${CLI_NAME}
        PUBLIC
        LibSignaling::LibSignaling)

find_package(LibWebRTC REQUIRED)
target_link_libraries(${CLI_NAME} PUBLIC LibWebRTCExtras::LibWebRTCExtras)
target_link_libraries(${CLI_NAME} PUBLIC LibWebRTC::LibWebRTC)

if (NOT MSVC)
    target_compile_options(${CLI_NAME} PRIVATE "-fno-rtti")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(${CLI_NAME}
            PUBLIC 
            "-framework CoreFoundation"
            "-framework QuartzCore"
            "-framework Cocoa"
            "-framework CoreAudio"
            "-framework AudioToolbox"
            cxxopts
            aws-cpp-sdk-core
            aws-cpp-sdk-monitoring
            # OpenSSL::SSL
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(
            ${CLI_NAME}
            PUBLIC pthread
            PUBLIC X11
            PUBLIC xcb
            PUBLIC glib-2.0
            PUBLIC dl
            PUBLIC cap
            PUBLIC z
            aws-cpp-sdk-core
            aws-cpp-sdk-monitoring
            PUBLIC cxxopts)
endif()
