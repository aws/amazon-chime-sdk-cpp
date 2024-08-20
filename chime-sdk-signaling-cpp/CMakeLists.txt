cmake_minimum_required(VERSION 3.17...3.20)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_NAME amazon-chime-signaling-sdk-cpp)

set(SIGNALING_LANGUAGES C CXX)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    list(APPEND SIGNALING_LANGUAGES OBJC)
endif()

project(
        AmazonChimeSignalingSdkCpp
        VERSION 0.1.0
        DESCRIPTION "Library for Amazon Chime's C++ Signaling SDK"
        LANGUAGES ${SIGNALING_LANGUAGES})

# list of options available
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(BUILD_TEST "Build tests" ON)

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined"
            )
    set(CMAKE_LINKER_FLAGS_DEBUG
            "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined")
    set(CMAKE_C_FLAGS_DEBUG
            "${CMAKE_C_FLAGS_DEBUG} -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined"
            )
endif()

configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_SOURCE_DIR}/src/version.h"
)

# set flag to build in different env. Default to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(
        STATUS
        "CMakeLists.txt - Current variables:\nCURRENT_BUILD_TYPE: ${CMAKE_BUILD_TYPE}"
        "\nENABLE_ASAN: ${ENABLE_ASAN}"
        "\nBUILD_TEST: ${BUILD_TEST}"
)

set(SDK_HEADERS
        src/audio_video/local_audio_configuration.h
        src/audio_video/remote_video_source_info.h
        src/transport/signaling_transport.h
        src/signaling/default_signaling_client.h
        src/transport/signaling_transport_configuration.h
        src/transport/websocket_signaling_transport.h
        src/session/turn_credentials.h
        src/signaling/signaling_client.h
        src/signaling/signaling_client_observer.h
        src/signaling/signaling_client_configuration.h
        src/signaling/signaling_client_status.h
        src/signaling/signaling_client_status.h
        src/utils/attendee.h)

set(SDK_SRC
        src/websocket/certs/trusted.cc
        src/audio_video/default_audio_frame_adapter.cc
        src/transport/default_signaling_transport_factory.cc
        src/transport/websocket_signaling_transport.cc
        src/signaling/default_signaling_client.cc
        src/signaling/default_signaling_client_factory.cc
        src/utils/logging.cc
        src/websocket/libwebsockets_websocket.cc
        src/websocket/default_websocket_factory.cc
        src/proto/video_control_sdk.proto
        )

add_library(${LIBRARY_NAME} "${SDK_HEADERS}" "${SDK_SRC}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(FetchContent)


# Libwebsocket dependency

FetchContent_Declare(
        libwebsockets
        GIT_REPOSITORY https://github.com/warmcat/libwebsockets.git
        GIT_TAG        v4.3-stable
)

set(LWS_CTEST_INTERNET_AVAILABLE OFF CACHE BOOL "enable tests for proto" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "Don't build the libwebsocket-test-apps" FORCE)
set(LWS_WITHOUT_TEST_SERVER ON CACHE BOOL "Don't build the libwebsocket-test-apps" FORCE)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON CACHE BOOL "Don't build the libwebsocket-test-apps" FORCE)
set(LWS_WITHOUT_TEST_PING ON CACHE BOOL "Don't build the libwebsocket-test-apps" FORCE)
set(LWS_WITHOUT_TEST_CLIENT ON CACHE BOOL "Don't build the libwebsocket-test-apps" FORCE)
FetchContent_MakeAvailable(libwebsockets)

target_link_libraries(${LIBRARY_NAME} PRIVATE websockets)

# Protobuf dependency

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    # Revert to version before Abseil dependency as we haven't found a workaround for
    # issues similar to https://github.com/protocolbuffers/protobuf/issues/12637
    GIT_TAG        v3.21.12
)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable tests for protobuf" FORCE)
FetchContent_MakeAvailable(protobuf)

target_link_libraries(${LIBRARY_NAME} PRIVATE protobuf::libprotobuf)
target_include_directories(${LIBRARY_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})

# Generate .proto files with the current version of protobuf. These are not
# added to SCM because in the future we will allow custom protobuf usage
# i.e. to reuse the one in a libwebrtc build as to not duplicate symbols

# Newer versions of protobuf include protobuf-generate.cmake. For now we use the one bundled
# with cmake
include(FindProtobuf)
set(PROTO_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/src/proto")
protobuf_generate(
    TARGET ${LIBRARY_NAME}
)
# Include generated *.pb.h files. Since we do not upload headers to source control
# we 'include' them without a path (e.g. '#include "video_control_sdk.pb.h'). Since
# we expose these classes and types in external headers, we need to expose the protobuf
# headers as well.
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROTO_OUTPUT_PATH})
# Additional include since the generated source file will reference the full path
# of the generated header.
target_include_directories(${LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Project configuration

# Include src directory so that we can avoid .. in the header includes
target_include_directories(${LIBRARY_NAME}
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Expose include paths for upstream projects
target_include_directories(
        ${LIBRARY_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_17)
set_target_properties(${LIBRARY_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Add stack smashing protection. MSVC uses /GS which is enabled by default. See
# https://docs.microsoft.com/en-us/cpp/build/reference/gs-buffer-security-check?view=msvc-160
if(NOT MSVC)
    target_compile_options(${LIBRARY_NAME} PRIVATE "-fstack-protector")
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_options(${LIBRARY_NAME} PUBLIC "-ObjC")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Set Runtime Library as MT to use static library
    set_property(
            TARGET ${LIBRARY_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# For downstream projects that do not use CMake, we output static
# libraries that combine all of the dependencies.

set(SDK_BINARY_LIB_NAME "libamazon_chime_signaling_sdk")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SDK_BINARY_NAME "${SDK_BINARY_LIB_NAME}.a")

    add_custom_command(
            TARGET ${LIBRARY_NAME}
            POST_BUILD
            COMMAND ar -x $<TARGET_FILE:${LIBRARY_NAME}>
            COMMAND ar -x $<TARGET_FILE:protobuf::libprotobuf>
            COMMAND ar -x $<TARGET_FILE:websockets>
            COMMAND ar -qc ${SDK_BINARY_NAME} *.o
    )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SDK_BINARY_NAME "${SDK_BINARY_LIB_NAME}.a")
    add_custom_command(
            TARGET ${LIBRARY_NAME}
            POST_BUILD
            COMMAND
            libtool -static -o ${SDK_BINARY_NAME}
            $<TARGET_FILE:${LIBRARY_NAME}>
            $<TARGET_FILE:protobuf::libprotobuf>
            $<TARGET_FILE:websockets>
            $<TARGET_FILE:websockets_shared>
    )
endif()

if (BUILD_TEST)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()
