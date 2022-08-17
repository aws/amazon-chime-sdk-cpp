# For find_package_handle_standard_args function
include(FindPackageHandleStandardArgs)

message(
        DEBUG
        "\nFindLibSignaling.cmake - Current variables:\nCHIME_SIGNAL_SRC: ${CHIME_SIGNAL_SRC}\n"
)

list(APPEND LIB_SIGNAL_INCLUDE_DIR "${CHIME_SIGNAL_SRC}/src")
list(APPEND LIB_SIGNAL_INCLUDE_DIR "${CHIME_SIGNAL_SRC}/build/_deps/libwebsockets-src/include")

# Location for built library
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_library(
            LIB_SIGNAL_LIBRARY
            NAMES libamazon_chime_signaling_sdk.a
            HINTS "${CHIME_SIGNAL_SRC}/build"
    )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    find_library(
            LIB_SIGNAL_LIBRARY
            NAMES libamazon_chime_signaling_sdk.lib
            HINTS "${CHIME_SIGNAL_SRC}/build"
    )
endif()

# Location for header files. All located under include for this dependency
find_package_handle_standard_args(
        LibSignaling DEFAULT_MSG LIB_SIGNAL_LIBRARY LIB_SIGNAL_INCLUDE_DIR)

mark_as_advanced(LIB_SIGNAL_LIBRARY LIB_SIGNAL_INCLUDE_DIR)

if(LIBSIGNALING_FOUND AND NOT TARGET LibSignaling::LibSignaling)
    add_library(LibSignaling::LibSignaling STATIC IMPORTED)
    set_target_properties(
            LibSignaling::LibSignaling
            PROPERTIES IMPORTED_LOCATION ${LIB_SIGNAL_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES "${LIB_SIGNAL_INCLUDE_DIR}")
endif()
