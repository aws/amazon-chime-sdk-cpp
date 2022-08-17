# This is an example CMake file that builds and run unit tests
# Unit tests will be under build folder
# You will need to cd build && ctest
cmake_minimum_required(VERSION 3.17...3.20)

project(AmazonChimeSdkCppUnitTest VERSION 0.1.0
        DESCRIPTION "Unit test for Amazon Signaling Chime's C++ Media SDK"
        LANGUAGES CXX C)

## GoogleTest requires at least C++ 11
set(CMAKE_CXX_STANDARD 17)

set(UNIT_TEST_NAME unit_test)
set(UNIT_TEST_NAME ${UNIT_TEST_NAME} PARENT_SCOPE)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

## Tell CMake where to look for the Find*.cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")


## Add googletest framework from github
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

FetchContent_MakeAvailable(googletest)

## Add unit tests files to compile
add_executable(${UNIT_TEST_NAME}
        main.cc
        src/signaling/default_signaling_client_tests.cc
        src/transport/websocket_signaling_transport_tests.cc
        src/utils/sdp_utils_tests.cc
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    ## Set Runtime Library as MT to use static library
    set_property(TARGET ${UNIT_TEST_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT\"")

    if (CMAKE_BUILD_TYPE MATCHES Debug)
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"MSVCRTD\"")
    endif()
endif()

## Compile with C++ 17
target_compile_features(${UNIT_TEST_NAME} PUBLIC cxx_std_17)
set_target_properties(${UNIT_TEST_NAME} PROPERTIES CXX_EXTENSIONS OFF)

## Add library necessary for unit test
target_include_directories(${UNIT_TEST_NAME}
        PRIVATE ${TEST_INCLUDE_DIR}
        PRIVATE ${CMAKE_BINARY_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        )

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

if (NOT MSVC)
    target_compile_options(${UNIT_TEST_NAME} PRIVATE "-fno-rtti")
endif()

target_link_libraries(
        ${UNIT_TEST_NAME}
        PRIVATE
        ${LIBRARY_NAME}
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(
            ${UNIT_TEST_NAME}
            PRIVATE
            -lpthread
            -lm
    )
endif()

target_link_libraries(
        ${UNIT_TEST_NAME}
        PRIVATE
        gtest
        gmock
        protobuf::libprotobuf
)

file(COPY example_files/example_sdp.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


## Add executable to googletest
include(GoogleTest)

## Default timeout is 5s and will sometimes time out
gtest_discover_tests(${UNIT_TEST_NAME} DISCOVERY_TIMEOUT 30)
