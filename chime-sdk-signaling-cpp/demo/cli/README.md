# Amazon Chime SDK for C++ Signaling Client Demo Application

This documentation was last tested with Ubuntu 24.04 LTS (x86_64) and M128 `libwebrtc`. It should be compatable with later versions of `libwebrtc`, and the commands should translate trivially to other *modern* platforms (e.g. Amazon Linux 2023).

It is recommended to use the latest AMIs possible as `libwebrtc` does not attempt to maintain backwards compatability.

While we will keep a regular cadence of updating this demo, please cut an issue (or make a PR) if it fails on the latest version of `libwebrtc` or the latest version of your build platform. We may not be able to provide support for older platforms (e.g AL2) which may require pinning `libwebrtc` and `depot_tools` versions, or linking against Chromium provided STL in downstream builds.

## Building Demo Application

### 1. Create and Log Into Ubuntu (x86_64) EC2 Instance

Open the EC2 console for your account, and click **Launch Instances**. Select the [Ubuntu](https://ubuntu.com/aws) AMI, and pick a powerful enough instance (e.g. 'c7i.8xlarge') that builds will not take too long. Add at least 100 GB of storage for the `libwebrtc` build (note the built application will be much smaller in size).

After the instance is launched, you should be able to select the instance in the EC2 console, click 'Connect', and follow instructions to gain access to the instance.

### 2. Setup workspace

The rest of the tutorial will assume `CHIME_DEMO_DIRECTORY` is set.

```shell
mkdir chime-sdk-signaling-cpp-demo
cd chime-sdk-signaling-cpp-demo
# You may want to add something similar with the full path
# to your '.bash_profile', '.zshrc', etc.
export CHIME_DEMO_DIRECTORY=$(pwd)
```

### 2. Fetch `depot_tools` and `libwebrtc`

This is excerpted from [WebRTC Development Prerequisite](https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/). The `gclient sync` flags come from [Chromium documentation](https://chromium.googlesource.com/chromium/src.git/+/HEAD/docs/building_old_revisions.md#sync-dependencies).

```shell
cd $CHIME_DEMO_DIRECTORY
mkdir webrtc-build
cd webrtc-build

# Pull depot_tools. This package does not need to be built
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
# The rest of instructions assume that depot_tools is in PATH. You
# may want to add this to your '.bash_profile', '.zshrc', etc.
export PATH=$CHIME_DEMO_DIRECTORY/webrtc-build/depot_tools:$PATH

fetch --nohooks webrtc
# We use a slightly roundabout way of reverting to a specific version
# of libwebrtc.
cd src
git checkout -b M128 refs/remotes/branch-heads/6613
cd ..
gclient sync -D --force --reset --with_branch_heads
cd src
# Must be done regardless of version
./build/install-build-deps.sh
```

### 3. Build `libwebrtc`

We do not build tests because they often do not interact well with `use_custom_libcxx`. Since the demo uses some test components, we will extend the build files to support a target that includes what we need.

```shell
cd $CHIME_DEMO_DIRECTORY/webrtc-build/src

cat <<EOL >> BUILD.gn
rtc_static_library("webrtc_extras") {
  visibility = [ "//:default" ]
  sources = []
  complete_static_lib = true
  suppressed_configs += [ "//build/config/compiler:thin_archive" ]

  deps = [
    "api:enable_media_with_defaults",
    "modules/audio_device:test_audio_device_module",
  ]
}
EOL
```

We can then build our static libraries. The most important flag below is `use_custom_libcxx`. `libwebrtc`, by default, builds with a prebuilt `clang` and `libc++`. The STL usage may cause downstream issues, so `use_custom_libcxx=false` can disable that behavior.

```shell
cd $CHIME_DEMO_DIRECTORY/webrtc-build/src

# Explanation of some additional flags:
# * Set `rtc_use_h264=false` to avoid building codecs
#   that may have problematic licenses.
# * Set 'rtc_enable_protobuf=false' to avoid downstream duplicate symbol issues.
# * Set 'rtc_include_pulse_audio=false' to maximize portability.
# * Set 'rtc_include_tests=false' avoids building some files with STL incompatibilities when 'use_custom_libcxx=false' is set.
#
# Note you may want to set 'is_debug=false' for production builds
gn_args='target_os="linux" 
        rtc_use_h264=false
        rtc_include_tests=false 
        use_custom_libcxx=false
        rtc_enable_protobuf=false
        rtc_include_pulse_audio=false'
gn gen out/Default --args="${gn_args}"
ninja -C out/Default
# Build additional module for test audio devices
ninja -C out/Default webrtc_extras
# We rename 'src' to 'webrtc' so that our files can
# prefix includes with 'webrtc' so we avoid filename collisions
# and so we can tell apart files from the library easily
#
# Note if you want to run `gclient sync` again in the future
# you will need to change the name back to src
cd .. && mv src webrtc
```

### 4. Install CMake and Ninja

Install CMake that is used for building Amazon Chime SDK for C++ Signaling Client. This is easiest using your package manager, but you can also [download a release](https://cmake.org/download/) or [build from source](https://gitlab.kitware.com/cmake/cmake). Additionally, we will use [Ninja](https://ninja-build.org/) to build a bit faster, though you can just omit `-GNinja` in later commands to use Makefiles.

```shell
sudo apt install cmake -y
sudo apt install ninja-build -y
```

### 5. Pull and build C++ SDK Signaling Client

To avoid downstream linking issues, and avoid relying on the often broken `rtc_build_ssl=false` flag, we simply use the `BoringSSL` implemenation bundled with the current version of libwebrtc. Note that the library currently has a small incompatability with `libwebsockets` that can be avoided with `LWS_HAVE_OPENSSL_STACK=OFF`.

```shell
cd $CHIME_DEMO_DIRECTORY
git clone https://github.com/aws/amazon-chime-sdk-cpp.git
cd amazon-chime-sdk-cpp/chime-sdk-signaling-cpp

# Note this is statically linked into the main libwebrtc library!
export BORING_SSL_LIB=$CHIME_DEMO_DIRECTORY/webrtc-build/webrtc/out/Default/obj/libwebrtc.a
export BORING_SSL_INCLUDE_DIR=$CHIME_DEMO_DIRECTORY/webrtc-build/webrtc/third_party/boringssl/src/include
cmake -S . -B build \
    -DLWS_OPENSSL_LIBRARIES=$BORING_SSL_LIB \
    -DLWS_OPENSSL_INCLUDE_DIRS=$BORING_SSL_INCLUDE_DIR \
    -DLWS_WITH_BORINGSSL=ON \
    -DLWS_HAVE_OPENSSL_STACK=OFF \
    -GNinja
cmake --build build
```

If during the generation phase you see something like the following:

```txt
-- Looking for HMAC_CTX_new
-- Looking for HMAC_CTX_new - not found
```

you likely have not configured your SSL libraries correctly.

### 6. Build C++ SDK Signaling Client Demo

This is relatively straightforward assuming the above steps have been completed correctly.

```shell
cd $CHIME_DEMO_DIRECTORY/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/demo/cli
cmake -S . -B build -GNinja
cmake --build build
```

## Running the Demo Application

The binary `my_cli` requires values collected from [CreateMeeting](https://docs.aws.amazon.com/chime-sdk/latest/APIReference/API_meeting-chime_CreateMeeting.html) and [CreateAttendee](https://docs.aws.amazon.com/chime-sdk/latest/APIReference/API_meeting-chime_CreateAttendee.html). 

It is recommended to run a [serverless demo](https://github.com/aws/amazon-chime-sdk-js/blob/main/demos/serverless/README.md), since that allows web clients to join at the same time. `run_with_serverless_demo.sh` will query a deployed serverless demo for credentials and use those instantiate the CLI.

```shell
cd $CHIME_DEMO_DIRECTORY/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp

# Will be outputted by deployment, e.g. https://XXXXXX.execute-api.us-east-1.amazonaws.com/Prod/
export SERVERLESS_DEMO_URL= # ... 
./run_with_serverless_demo.sh --meeting test-meeting --attendee cli-attendee --url $SERVERLESS_DEMO_URL
```

There is a keypress monitor that supports the following commands. You must press enter for the command to be invoked:

* `q`: Quit
* `c`: Enable test video capture
* `d`: Send a test data message