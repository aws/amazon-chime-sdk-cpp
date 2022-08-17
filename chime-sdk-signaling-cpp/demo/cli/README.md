# Amazon Chime SDK for C++ Signaling Client Demo Application

> Note: This documentation is written with Amazon Linux 2 and M99 WebRTC.

## Building Demo Application

### 1. Create Amazon Linux 2 EC2 Instance 

Use [EC2](https://aws.amazon.com/ec2/) to create [AL2](https://aws.amazon.com/amazon-linux-2/?amazon-linux-whats-new.sort-by=item.additionalFields.postDateTime&amazon-linux-whats-new.sort-order=desc) instance.

Make sure to give at least 100 GB of storage.

### 2. Install necessary library for WebRTC

Most of libraries are excerpted from [WebRTC Development Prerequisite](https://chromium.googlesource.com/chromium/src/+/master/docs/linux/build_instructions.md)

```
sudo yum install bzip2 pkgconfig alsa-lib-devel atk-devel -y &&\
sudo yum install bison binutils brlapi-devel bluez-libs-devel bzip2-devel cairo-devel -y &&\
sudo yum install cups-devel dbus-devel dbus-glib-devel expat-devel fontconfig-devel -y &&\
sudo yum install freetype-devel gcc-c++ glib2-devel glibc.i686 gperf glib2-devel -y &&\
sudo yum install gtk3-devel libatomic libcap-devel libffi-devel -y &&\
sudo yum install libgcc.i686 libgnome-keyring-devel libjpeg-devel libstdc++.i686 libX11-devel -y &&\
sudo yum install libXScrnSaver-devel libXtst-devel libxkbcommon-x11-devel ncurses-compat-libs -y &&\
sudo yum install pulseaudio-libs-devel zlib.i686 httpd mod_ssl php php-cli python-psutil -y &&\
sudo yum install xorg-x11-server-Xvfb -y &&\
sudo yum install pam-devel pango-devel pciutils-devel -y &&\
sudo yum install clang clang-devel llvm-devel -y
```

### 3. Install CMake

```
wget https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3.tar.gz &&\
tar -xzf cmake-3.22.3.tar.gz &&\
cd cmake-3.22.3 &&\
./bootstrap &&\
make &&\
sudo make install
```

### 4. Install Libcxx

For demo purpose, we’ll use 13.0.1 RC for LLVM.

```
cd ~ &&\
git clone https://github.com/llvm/llvm-project.git &&\
cd llvm-project &&\
git checkout llvmorg-13.0.1-rc3
```

Build libcxx without libcxxabi
```
cd libcxx &&\
# Build without libcxxabi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ &&\
cmake --build build &&\
cd build &&\
sudo make install &&\
cd ../..
```

Build libcxxabi with libcxx
```
cd libcxxabi &&\
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLIBCXXABI_LIBCXX_INCLUDES=../libcxx/build/include/c++/v1 &&\ 
cmake --build build &&\
cd build &&\
sudo make install &&\
cd ../..
```

Build libcxx with libcxxabi
```
cd libcxx &&\
rm -rf build &&\
cmake -S . -B build . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DLIBCXX_CXX_ABI=libcxxabi -DLIBCXX_CXX_ABI_INCLUDE_PATHS=../libcxxabi/include &&\
cmake --build build &&\
cd build &&\
sudo make install &&\
cd ../..
```

(Optional) Enable ASAN Support
```
cd compiler-rt &&\
# You can set install path to your clang
cmake -S . -B build . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ &&\
cmake --build build &&\
cd build && sudo make install
# If you didn't set install path to your clang
cd /usr/lib64/clang/11.1.0/lib &&\
sudo ln -s /usr/lib/linux linux
```

### 5. Clone necessary library and build

```
mkdir webrtc-build && cd webrtc-build &&\
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git &&\
export PATH=$HOME/webrtc-build/depot_tools:$PATH &&\
fetch --nohooks webrtc &&\
gclient sync &&\
gclient sync -D &&\
cd src &&\
git checkout -b _my__m99_branch refs/remotes/branch-heads/4844 &&\
cd .. &&\
gclient sync &&\
gclient sync -D &&\
cd src &&\
gn_args=is_debug=false \
        rtc_include_tests=false \
        target_os="linux" \
        use_glib=true \
        libcxx_abi_unstable=false \
        rtc_use_h264=false \
        rtc_enable_libevent=false \
        libcxx_is_shared=true \
        rtc_use_dummy_audio_file_devices=true \
        rtc_include_pulse_audio=false ffmpeg_branding="Chrome" &&\
gn gen out/Default --args=$gn_args &&\
ninja -C out/Default buildtools/third_party/libc++:libc++ -v &&\
sudo cp out/Default/libc++.so /usr/lib64 &&\
ninja -C out/Default -v &&\
cd .. && mv src webrtc
```

### 6. Build C++ SDK signaling client

```
git clone https://github.com/aws/amazon-chime-sdk-cpp.git &&\
export VANILLA_WEBRTC_SRC=$HOME/webrtc-build &&\
export BORING_SSL_LIBS=$VANILLA_WEBRTC_SRC/webrtc/build/linux/debian_sid_amd64-sysroot/usr/lib/x86_64-linux-gnu &&\
export BORING_SSL_INCLUDE_DIR=$VANILLA_WEBRTC_SRC/webrtc/third_party/boringssl/src/include &&\
export TOOLCHAIN_FILE=$HOME/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/cmake/toolchains/LinuxClang.cmake &&\
cd ~/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp &&\
rm -rf build && cmake -S . -B build -DLWS_OPENSSL_LIBRARIES="${BORING_SSL_LIBS}/libssl.a;${BORING_SSL_LIBS}/libcrypto.a" -DLWS_OPENSSL_INCLUDE_DIRS=$BORING_SSL_INCLUDE_DIR -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" &&\ 
cmake --build build &&\
cd demo/cli &&\
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

