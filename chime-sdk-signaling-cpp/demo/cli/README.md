# Amazon Chime SDK for C++ Signaling Client Demo Application

> Note: This documentation is written with Ubuntu 18.04 (x86_64) and M99 WebRTC.

## Building Demo Application

### 1. Create Ubuntu 18.04 (x86_64) EC2 Instance 

Use [EC2](https://aws.amazon.com/ec2/) to create [Ubuntu](https://ubuntu.com/aws) instance.

Make sure to give at least 100 GB of storage.

### 2. Install necessary library for WebRTC

This is excerpted from [WebRTC Development Prerequisite](https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/)

```
cd ~ && mkdir webrtc-build && cd webrtc-build &&\
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git &&\
export PATH=$HOME/webrtc-build/depot_tools:$PATH &&\
fetch --nohooks webrtc &&\
cd src &&\
git checkout -b _my__m99_branch refs/remotes/branch-heads/4844 &&\
cd .. &&\
gclient sync &&\
gclient sync -D &&\
cd src &&\
./build/install-build-deps.sh
```

### 3. Install CMake
Install CMake that is used for building Amazon Chime SDK for C++ signaling client.
```
cd ~ &&\
wget https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3.tar.gz &&\
tar -xzf cmake-3.22.3.tar.gz &&\
cd cmake-3.22.3 &&\
./bootstrap &&\
make &&\
sudo make install
```
### 4. Build the WebRTC
```
export PATH=$HOME/webrtc-build/depot_tools:$PATH &&\
cd $HOME/webrtc-build/src &&\
gn_args='is_debug=false
        rtc_include_tests=false 
        target_os="linux" 
        use_glib=true 
        libcxx_abi_unstable=false 
        rtc_use_h264=false 
        rtc_enable_libevent=false 
        libcxx_is_shared=true 
        rtc_use_dummy_audio_file_devices=true 
        rtc_include_pulse_audio=false ffmpeg_branding="Chrome"' &&\
gn gen out/Default --args="${gn_args}" &&\
ninja -C out/Default buildtools/third_party/libc++:libc++ -v &&\
sudo cp out/Default/libc++.so /usr/lib/ &&\
ninja -C out/Default -v &&\
cd .. && mv src webrtc
```

### 5. Use WebRTC Clang for build
Use the Clang from WebRTC.
```
git clone https://github.com/aws/amazon-chime-sdk-cpp.git &&
cd ~/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp &&\
vim cmake/toolchains/LinuxClang.cmake
```

Change `LinuxClang.cmake` to following
```
set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER  /home/ubuntu/webrtc-build/webrtc/third_party/llvm-build/Release+Asserts/bin/clang)
set(CMAKE_CXX_COMPILER /home/ubuntu/webrtc-build/webrtc/third_party/llvm-build/Release+Asserts/bin/clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

include_directories(SYSTEM /home/ubuntu/webrtc-build/webrtc/buildtools/third_party/libc++/trunk/include /home/ubuntu/webrtc-build/webrtc/third_party/llvm-build/Release+Asserts/lib/clang/14.0.0/include/ /home/ubuntu/webrtc-build/webrtc/buildtools/third_party/libc++/ /home/ubuntu/webrtc-build/webrtc/buildtools/third_party/libc++abi/trunk/include)
```


### 6. Build C++ SDK signaling client

```
export VANILLA_WEBRTC_SRC=$HOME/webrtc-build &&\
export BORING_SSL_LIBS=$VANILLA_WEBRTC_SRC/webrtc/build/linux/debian_sid_amd64-sysroot/usr/lib/x86_64-linux-gnu &&\
export BORING_SSL_INCLUDE_DIR=$VANILLA_WEBRTC_SRC/webrtc/third_party/boringssl/src/include &&\
export TOOLCHAIN_FILE=$HOME/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/cmake/toolchains/LinuxClang.cmake &&\
cd ~/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp &&\
rm -rf build && cmake -S . -B build -DLWS_OPENSSL_LIBRARIES="${BORING_SSL_LIBS}/libssl.a;${BORING_SSL_LIBS}/libcrypto.a" -DLWS_OPENSSL_INCLUDE_DIRS=$BORING_SSL_INCLUDE_DIR -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" &&\ 
cmake --build build
```

### 7. Build C++ SDK signaling client Demo
```
cd $HOME/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/demo/cli &&\
rm -rf build && cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" &&\
cmake --build build
```

## Running the Demo Application
```
cd ~/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/build
# Arguments are retrieved from Chime AWS SDK: https://docs.aws.amazon.com/chime/latest/APIReference/API_CreateMeetingWithAttendees.html
# or you can use curl -XPOST "https://xxxxx.xxxxx.xxx.com/Prod/join?title={MEETING_NAME}&name={ATTENDEE_NAME}&region={MEETING_REGION}" from [Serverless Demo](https://github.com/aws/amazon-chime-sdk-js/tree/main/demos/serverless)
# ex: curl -XPOST "https://xxxxx.xxxxx.xxx.com/Prod/join?title=<meeting-title>&name=<attendee-name>&region=us-east-1"
./my_cli --attendee_id=<ATTENDEE_ID> --audio_host_url=<AUDIO_HOST_URL> --external_meeting_id=<EXTERNAL_MEETINIG_ID> --external_user_id=<EXTERNAL_USER_ID> --join_token=<JOIN_TOKEN> --log_level=<LOG_LEVEL> --meeting_id=<MEETING_ID> --signaling_url=<SIGNALING_URL> --send_audio_file_name=<SEND_AUDIO_FILE>
# Ex: ./my_cli --attendee_id 1234567-1234-1234-1234-123456789012 --audio_host_url aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.k.m3.ue1.app.chime.aws:3478 --external_meeting_id example --external_user_id af9de7ed#eee --join_token xxxxx --log_level error --meeting_id 1234567-1234-1234-1234-123456789012 --signaling_url wss://signal.m3.ue1.app.chime.aws/control/1234567-1234-1234-1234-123456789012 --send_audio_file_name "./hello.pcm"
```

send_audio_file_name requires a stereo, 16 bit, 48kHz, raw pcm file. The following steps can be used to get a public domain pcm file on mac:

1. Download mp4 from here: https://musopen.org/music/2567-symphony-no-5-in-c-minor-op-67/?fbclid=IwAR3ajy-GQyg0_Y18KmNJpmh2yyMQG39yCAiVTECjtpmTs5bFj7ZcXkR3984
2. brew install ffmpeg
3. ffmpeg -i ~/Downloads/sample_mp3_file.mp3 -f s16le -sample_rate 48000 -acodec pcm_s16le Beethoven_Symphony_no5_48k_16bit.pcm
    1. You can test the audio with: ffplay -autoexit -f s32le -sample_rate 48000 Beethoven_Symphony_no5_48k_16bit.pcm
4. Move this to `amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/demo/cli/media_in` on your linux machine.

