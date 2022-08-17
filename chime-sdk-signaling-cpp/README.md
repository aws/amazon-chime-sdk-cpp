# Amazon Chime SDK for C++ Signaling Client
[Amazon Chime SDK Project Board](https://aws.github.io/amazon-chime-sdk-js/modules/projectboard.html)

>Note: main branch contains bleeding-edge changes that might be breaking. 

## Build video calling, audio calling, and screen sharing applications powered by Amazon Chime.

With the Amazon Chime SDK for C++ signaling client developers can compile on their preferred platforms and establish a signaling connection to the Amazon Chime SDK media service. Developers have the ability to use a custom WebRTC implementation to send and receive audio, video, screen share, and data messages to Amazon Chime SDK meetings.

We also have an [Amazon Chime SDK Project Board](https://aws.github.io/amazon-chime-sdk-js/modules/projectboard.html) where you can find community requests and their statuses.

To get started, see the following resources:

* [Amazon Chime](https://aws.amazon.com/chime)
* [Amazon Chime Developer Guide](https://docs.aws.amazon.com/chime/latest/dg/what-is-chime.html)
* [Amazon Chime SDK API Reference](http://docs.aws.amazon.com/chime/latest/APIReference/Welcome.html)
* [SDK Documentation](https://aws.github.io/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/)

## Setup

The Amazon Chime SDK for C++ signaling client requires that builders have OpenSSL/BoringSSL.

### Building C++ SDK signaling client


#### Linux
On the chime-sdk-signaling-cpp, run the following command

```
cmake -S . -B build -DLWS_OPENSSL_LIBRARIES="<libssl-location>/libssl.a;<libcrypto-location>/libcrypto.a" -DLWS_OPENSSL_INCLUDE_DIRS=<ssl-header-location> -DCMAKE_TOOLCHAIN_FILE="./cmake/toolchains/LinuxClang.cmake"
cmake --build build
```

## Running the demo app

To run the demo application, see [here](demo/cli/README.md)

## Reporting a suspected vulnerability

If you discover a potential security issue in this project we ask that you notify AWS/Amazon Security via our [vulnerability reporting page](https://aws.amazon.com/security/vulnerability-reporting/). Please do not create a public GitHub issue.

## Usage
  - [Create a Signaling Client](#create-a-signaling-client)
  - [Starting Signaling Client](#starting-signaling-client)
  - [Adding Local Audio/Video](#adding-local-audiovideo)
  - [Adding Remote Video when Available](#adding-remote-video-when-available)
  - [Getting Real Time Updates](#getting-real-time-updates)
  - [Stopping Signaling Client](#stopping-signaling-client)

### Create a Signaling Client

From server, you’ll receive meeting info:
`aws chime create-meeting-with-attendees --client-request-token `uuidgen` --attendees ....`

```
MeetingSessionConfiguration meeting_configuration = // create configuration with given data
SignalingClientConfiguration signaling_client_configuration;
signaling_configuration.meeting_configuration = meeting_configuration;
DefaultSignalingDependencies signaling_dependencies {};
std::unique_ptr<SignalingClient> signaling_client = DefaultSignalingClientFactory::CreateSignalingClient(signaling_client_configuration, std::move(signaling_dependencies));
```

### Starting Signaling Client

```
class MySignalingObserver: public SignalingClientObserver, public webrtc::CreateSessionDescriptionObserver, public webrtc::PeerConnectionObserver, public webrtc::SetSessionDescriptionObserver {
  // other methods are omitted for brevity
  virtual void OnSignalingStarted(const SignalingClientStartInfo& join_info) override {
    // Connection has been established.
    // if no peer connection, setup new peer connection here.
    auto credentials = join_info.credentials;
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    webrtc::PeerConnectionInterface::IceServer server;
    server.urls = credentials.uris;
    server.username = credentials.username;
    server.password = credentials.password;
    config.servers.push_back(server);
    config.type = webrtc::PeerConnectionInterface::IceTransportsType::kRelay;
    peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);
    // Update local transceivers
    // After peer connection is setup
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    // Changes based on your implementation
    options.num_simulcast_layers = 1;
    peer_connection_->CreateOffer(new rtc::RefCountedObject<MySignalingObserver>(this), options);
  };

  // CreateSessionDescriptionObserver
  virtual OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) override {
    peer_connection_->SetLocalDescription(new rtc::RefCountedObject<MySignalingObserver>(true, this), desc);
  }

  virtual void OnCreateSessionDescriptionFailure(const webrtc::RTCError& error) override {
    // handle failure
  }

  // SetSessionDescriptionObserver
  virtual void OnSuccess() { 
    // if turn candidate has been gathered
    SendUpdate();
  }

  virtual void OnFailure(webrtc::RTCError error) { 
    // handle failure
  }

  virtual void OnIceCandidate(const IceCandidateInterface* candidate) override {
    // check turn candidate has been gathered
    // if turn candidate has been gathered
    SendUpdate();
  }

  void SendUpdate() {
    // Assuming that AddLocalVideo/AddLocalAudio/UpdateLocalVideo/UpdateLocalAudio/UpdateRemoteVideoSubscriptions has been called appropriately. 
    signaling_client_v2_->SetLocalDescription(sdp_offer);
    if (!signaling_client_v2_->SendUpdates()) {
        // Unable to send...
        return;
    }
  }

virtual void OnRemoteDescriptionReceived(const std::string& sdp) override {
        // peer connection set remote description using sdp
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription (webrtc::SdpType::kAnswer, sdp, &error);
        if (!session_description) return;
        // Setup codecs based description
        peer_connection_->SetRemoteDescription(
        new rtc::RefCountedObject<MySignalingObserver>(false, this), session_description.release());
    }
}

signaling_client.AddSignalingObserver(new MySignalingObserver());

signaling_client.Start();
// If you do not want to spawn thread by us
// do
// while (true) { signaling_client.Poll() }
signaling_client.Run();
```

### Adding Local Audio/Video

```
// Add/Update video
LocalVideoConfiguration local_video_configuration {};
local_video_configuration.max_bitrate_kbps = 1200; // speed at which you want to set. 1200 should be enough fro HD.
std::string local_video_mid = // mid from sdp of builder's local video transceiver
signaling_client.AddOrUpdateLocalVideo(local_video_mid, local_video_configuration);

// Add/Update audio
LocalAudioConfiguration local_audio_configuration {};
local_audio_configuration.mute_state = MuteState::kMute; // or MuteState::kUnmute
session configuration
std::string local_audio_mid = // mid from sdp of builder's local audio transceiver
signaling_client.AddLocalAudio(local_audio_mid, local_audio_configuration);

// Create an offer from peer connection
std::string sdp;
const webrtc::SessionDescriptionInterface* local_desc = peer_connection_->pending_local_description();
local_desc->ToString(&sdp);

// SetLocalDescription
signaling_client.SetLocalDescription(sdp);
   
   
// SendUpdate()
if (!signaling_client.SendUpdates()) {

}
```

### Adding Remote Video when Available

```
class MySignalingObserver: public SignalingClientObserver, public webrtc::SetSessionDescriptionObserver {
    virtual void OnRemoteVideoSourcesAvailable(const std::vector<chime::RemoteVideoSourceInfo> &sources) override {       
        std::vector<string> removed;
        std::map<std::string, RemoteVideoSourceInfo> mid_to_remote_video_sources;
        // Update mid_to_remote_video_sources with remote video transceivers
        signaling_client.UpdateRemoteVideoSubscriptions(mid_to_remote_video_sources, removed);
        if (signaling_client.ShouldUpdateSDP()) {
        // Create an offer from peer connection
        std::string sdp;
        const webrtc::SessionDescriptionInterface* local_desc = peer_connection_->pending_local_description();
        local_desc->ToString(&sdp);

        // SetLocalDescription
        signaling_client.SetLocalDescription(sdp);
        
        
        // SendUpdate()
        if (!signaling_client.SendUpdates()) {

        }
    }
}
```

### Getting Real Time Updates

```
class MySignalingObserver: public SignalingClientObserver {
  /**
   * Invoked when an attendee has joined the meeting.
   * A newly joined attendee will receive this callback
   * for attendees that are currently in the meeting.
   *
   * @param attendee - The joined attendee information
   */
  virtual void OnAttendeeJoined(const Attendee& attendee) override {};
  /**
   * Invoked when an attendee has left the meeting.
   *
   * @param attendee - The left attendee information
   */
  virtual void OnAttendeeLeft(const Attendee& attendee) override {};
  /**
   * Invoked when an attendee has dropped the meeting due to network.
   *
   * @param attendee - The dropped attendee information
   */
  virtual void OnAttendeeDropped(const Attendee& attendee) override {};
  // Real time updates volume/signal/data message
  /**
   * Invoked when the updated volumes are available.
   *
   * @param updates - The list of updates, only contains volumes that changed
   */
  virtual void OnVolumeUpdates(const std::vector<VolumeUpdate>& updates)override {};
  /**
   * Invoked when the updated signal strengths are available.
   *
   * @param updates - The list of updates, only contains signal strength that changed
   */
  virtual void OnSignalStrengthChanges(const std::vector<SignalStrengthUpdate>& updates) override {};
  /**
   * Invoked when an attendee's audio has muted
   *
   * @param attendee - The muted attendee information
   */
  virtual void OnAttendeeAudioMuted(const Attendee& attendee) override {}
  /**
   * Invoked when an attendee's audio has unmuted.
   *
   * @param attendee - The unmuted attendee information
   */
  virtual void OnAttendeeAudioUnmuted(const Attendee& attendee) override {}
  /**
   * Invoked when new data messages are received.
   *
   * @param messages - Data messages that contain information about messages sent
   */
  virtual void OnDataMessageReceived(const std::vector<DataMessageReceived>& messages) override {}
  /**
   * Invoked when data messages failed to be sent.
   *
   * @param to_send_errors - Data message errors that contain information why it failed.
   */
  virtual void OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) override {};
}
```

### Stopping Signaling Client

```
class MySignalingObserver: public SignalingClientObserver {
  // other methods are omitted for brevity
  virtual void OnSignalingStopped(const SignalingStatus& status) {
    // Websocket/Everything else should be all destroyed by now.
    signaling_client.RemoveSignalingClientObserver(mySignalingObserver);
  };
}

signaling_client.Stop();
```