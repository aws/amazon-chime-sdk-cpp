# Setup Instructions for C++ SDK on Windows

### Prerequisite

This method has been tested with MSVC, so you would need to install [Visual Studio](https://visualstudio.microsoft.com/downloads/) (2017/2019). 

Install Windows C++ development when launching Visual Studio Installer.

### Integration

You can build either using Visual Studio or using CMake.

>NOTE: We currently support static library and x86 only

### Visual Studio Project

Set your environment to release

1. On Property Pages, go to **`C/C++`**→**`General`**→**`Additional Include Directories`** and provide header path to downloaded `include` folder.
2. On Property Pages, go to **`C/C++`**→**`Code Generation`**→**`Runtime Library`** and set **`Multi-threaded (/MT`**`)`. For Debug build, use **`Multi-threaded Debug (/MTd)`**
3. Go to **`Linker`→`General`**→**`Additional Library Directories`** and specify the **`libamazon-chime-sdk.lib`** path.
4. Go to **`Linker`**→**`Input`**→**`Additional Dependencies`** and add `**libamazon-chime-sdk.lib**` to it.
5. Add system libraries `crypt32.lib`, `secur32.lib`, `strmiids.lib`, `winmm.lib`,` msdmo.lib` by following step 3.
6.  Go to **`Linker`**→**`Input`**→**`Ignore Specific Default Libraries`** put **`LIBCMT`**. For Debug build, ignore **`MSVCRTD`** additionally.

>NOTE: If you want to ignore the warnings on pdb, Add `/ignore:4099 `Properties->Linker->Command Line

### CMake Project

**Folder structure**

This is roughly what your folder structure should look like to take advantage of this example. This example is not fully functional and only meant as a starting point. These CMake files must be carefully modified to fit your use case.

```
|   CMakeLists.txt
|   main.cc
|
+---cmake
|       FindLibAmazonChimeSdk.cmake
|
\---lib
    | +----bin  
    |         libamazon-chime-sdk.lib
    |
    \---include
        +---audiovideo
        |   +---audio
        |   +---video
        +---session
        |
        \---utils
            \---logger
```

 **CmakeLists.txt**

```
cmake_minimum_required(VERSION 3.17...3.20)
set(CMAKE_CXX_STANDARD 14)
project(MyDemoApp VERSION 0.1.0
                  DESCRIPTION "Demo Console using Chime SDK"
                  LANGUAGES CXX)
                            
## Take input parameters
set(WORKSPACE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../" CACHE INTERNAL "Path to the workspace containing this project and its internal dependencies")
message(STATUS "\nCmakeLists.txt - Current cache variables WORKSPACE_SRC: ${WORKSPACE_SRC}\n")

## Tell CMake where to look for the Find*.cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Build the C++ CLI
add_executable(my-demo main.cc)

## Set Runtime Library as MT to use static library
message(STATUS "Setting MSVC_RUNTIME_LIBRARY")
set_property(TARGET my-demo PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_features(my-demo PUBLIC cxx_std_14)
set_target_properties(my-demo PROPERTIES CXX_EXTENSIONS OFF)
## Add Chime C++ SDK dependency
find_package(libAmazonChimeSdk REQUIRED)

## Link dependencies together
target_link_libraries(my-demo
                      PUBLIC libAmazonChimeSdk::libAmazonChimeSdk
)

target_link_libraries(my-demo
                      PRIVATE crypt32.lib
                      PRIVATE secur32.lib
                      PRIVATE strmiids.lib
                      PRIVATE winmm.lib
                      PRIVATE msdmo.lib
)
SET(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"LIBCMT\")

## Include Chime Media SDK header files
target_include_directories(my-demo PUBLIC "${CMAKE_SOURCE_DIR}/lib/include")

## Copy file to build folder
add_custom_command(TARGET my-demo POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:my-demo>
                    ${CMAKE_CURRENT_SOURCE_DIR}/build
)
```

**cmake/FindLibAmazonChimeSdk.cmake**

```
# For find_package_handle_standard_args function
include(FindPackageHandleStandardArgs)

# Location for built library
set(LIBAMAZONCHIMESDK_LIBRARY "${CMAKE_SOURCE_DIR}/lib/bin/libamazon-chime-sdk.lib")

# Location for header files
set(LIBAMAZONCHIMESDK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/lib/include")

find_package_handle_standard_args(libAmazonChimeSdk DEFAULT_MSG
                                  LIBAMAZONCHIMESDK_LIBRARY
                                  LIBAMAZONCHIMESDK_INCLUDE_DIR)

mark_as_advanced(LIBAMAZONCHIMESDK_LIBRARY LIBAMAZONCHIMESDK_INCLUDE_DIR)

if (LIBAMAZONCHIMESDK_FOUND AND NOT TARGET libAmazonChimeSdk::libAmazonChimeSdk)
    add_library(libAmazonChimeSdk::libAmazonChimeSdk STATIC IMPORTED)
    set_target_properties(
        libAmazonChimeSdk::libAmazonChimeSdk
        PROPERTIES
            IMPORTED_LOCATION ${LIBAMAZONCHIMESDK_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES "${LIBAMAZONCHIMESDK_INCLUDE_DIR}")
endif()
```

 Run with `x86 Native Tools Command Prompt` that comes with MSVC.

```
mkdir build
cd build
# Add win32 platform
cmake -A Win32 ..
msbuild my-demo.vcxproj /p:Configuration=Release /p:Platform="Win32"
```



