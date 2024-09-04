mkdir chime-sdk-signaling-cpp-demo
cd chime-sdk-signaling-cpp-demo
export CHIME_SDK_DEMO_DIRECTORY=$(pwd)

cd $CHIME_SDK_DEMO_DIRECTORY
mkdir webrtc-build
cd webrtc-build

git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=$CHIME_SDK_DEMO_DIRECTORY/webrtc-build/depot_tools:$PATH

fetch --nohooks webrtc
cd src
git checkout -b M128 refs/remotes/branch-heads/6613
cd ..
gclient sync -D --force --reset --with_branch_heads
cd src
./build/install-build-deps.sh


cd $CHIME_SDK_DEMO_DIRECTORY/webrtc-build/src

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

cd $CHIME_SDK_DEMO_DIRECTORY/webrtc-build/src
gn_args='target_os="linux" 
        rtc_use_h264=false
        rtc_include_tests=false 
        use_custom_libcxx=false
        rtc_enable_protobuf=false
        rtc_include_pulse_audio=false'
gn gen out/Default --args="${gn_args}"
ninja -C out/Default
ninja -C out/Default webrtc_extras
cd .. && mv src webrtc


sudo apt install cmake -y
sudo apt install ninja-build -y


cd $CHIME_SDK_DEMO_DIRECTORY
git clone https://github.com/aws/amazon-chime-sdk-cpp.git
cd amazon-chime-sdk-cpp/chime-sdk-signaling-cpp

export BORING_SSL_LIB=$CHIME_SDK_DEMO_DIRECTORY/webrtc-build/webrtc/out/Default/obj/libwebrtc.a
export BORING_SSL_INCLUDE_DIR=$CHIME_SDK_DEMO_DIRECTORY/webrtc-build/webrtc/third_party/boringssl/src/include
cmake -S . -B build \
    -DLWS_OPENSSL_LIBRARIES=$BORING_SSL_LIB \
    -DLWS_OPENSSL_INCLUDE_DIRS=$BORING_SSL_INCLUDE_DIR \
    -DLWS_WITH_BORINGSSL=ON \
    -DLWS_HAVE_OPENSSL_STACK=OFF \
    -GNinja
cmake --build build

cd $CHIME_SDK_DEMO_DIRECTORY/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/demo/cli
cmake -S . -B build -GNinja
cmake --build build