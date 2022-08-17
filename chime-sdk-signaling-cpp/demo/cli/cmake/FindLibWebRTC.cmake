# For find_package_handle_standard_args function
include(FindPackageHandleStandardArgs)

message(DEBUG "\nFindLibWebRTC.cmake - Current variables:\nVANILLA_WEBRTC_SRC: ${VANILLA_WEBRTC_SRC}\n")

set(LIBWEBRTC_BINARY_PATH_SUFFIX "webrtc/out/Default/obj")

# Location for built library
find_library(
    LIBWEBRTC_LIBRARY
    NAMES libwebrtc.a
    HINTS ${VANILLA_WEBRTC_SRC}
    PATH_SUFFIXES ${LIBWEBRTC_BINARY_PATH_SUFFIX}
)

# Location for header files
# find_path also works but it searches one path at a time and we already know them
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/third_party/abseil-cpp")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/third_party/protobuf/src")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/third_party/libyuv/include")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/third_party/libsrtp/include")
list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/third_party/libsrtp/crypto/include")

if (APPLE)
    list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/sdk/objc")
    list(APPEND LIBWEBRTC_INCLUDE_DIR "${VANILLA_WEBRTC_SRC}/webrtc/sdk/objc/base")
endif (APPLE)

message(DEBUG "\nFindLibWebRTC.cmake - LIBWEBRTC_INCLUDE_DIR is ${LIBWEBRTC_INCLUDE_DIR}\n")

find_package_handle_standard_args(LibWebRTC DEFAULT_MSG
                                  LIBWEBRTC_LIBRARY
                                  LIBWEBRTC_INCLUDE_DIR)

mark_as_advanced(LIBWEBRTC_LIBRARY LIBWEBRTC_INCLUDE_DIR)

if (LIBWEBRTC_FOUND AND NOT TARGET LibWebRTC::LibWebRTC)
    add_library(LibWebRTC::LibWebRTC STATIC IMPORTED)
    set_target_properties(
        LibWebRTC::LibWebRTC
        PROPERTIES
            IMPORTED_LOCATION ${LIBWEBRTC_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES "${LIBWEBRTC_INCLUDE_DIR}")
endif()
