# Amazon Chime SDK for C++ Signaling Client

[Amazon Chime SDK Project Board](https://aws.github.io/amazon-chime-sdk-js/modules/projectboard.html)

## Build video calling, audio calling, and screen sharing applications powered by Amazon Chime.

With the Amazon Chime SDK for C++ Signaling Client developers can compile on their preferred platforms and establish a signaling connection to the Amazon Chime SDK media service. Developers can then integrate with their preferred WebRTC implementation to send and receive audio, video, screen share, and data messages to Amazon Chime SDK meetings.

We have an [Amazon Chime SDK Project Board](https://aws.github.io/amazon-chime-sdk-js/modules/projectboard.html) where you can find community requests and their statuses.

To get started, see the following resources:

* [Amazon Chime](https://aws.amazon.com/chime)
* [Amazon Chime Developer Guide](https://docs.aws.amazon.com/chime/latest/dg/what-is-chime.html)
* [Amazon Chime SDK API Reference](http://docs.aws.amazon.com/chime/latest/APIReference/Welcome.html)
* [SDK Documentation](https://aws.github.io/amazon-chime-sdk-cpp/chime-sdk-signaling-cpp/)

## Setup

The Amazon Chime SDK for C++ Signaling Client requires OpenSSL-based or mbedTLS libraries compatible with [libwebsockets](https://libwebsockets.org/). 

### Building

#### Linux

In the `chime-sdk-signaling-cpp` folder, run the following command to generate build files

```shell
cmake -S . -B build
```

The default build will check for SSL libraries in `/usr/lib/` and `/usr/include/`, however the location of e.g. `libssl.a` may vary. To set the location use `LWS_OPENSSL_LIBRARIES` and `LWS_OPENSSL_INCLUDE_DIRS` (both must be set at same time), e.g.:

```shell
export SSL_LIB_PATH=/usr/lib/x86_64-linux-gnu
export SSL_INCLUDE_DIR=/usr/include/x86_64-linux-gnu/openssl

cmake -S . -B build \
    -DLWS_OPENSSL_LIBRARIES="${SSL_LIB_PATH}/libssl.a;${SSL_LIB_PATH}/libcrypto.a" \
    -DLWS_OPENSSL_INCLUDE_DIRS=$SSL_INCLUDE_DIR  
```

You should determine if your WebRTC implementation is already linking to an OpenSSL implmentation and use the same one to avoid linking issues with this SDK.

To run the build

```shell
cmake --build build
```

### Running unit tests

Unit tests can be run using `ctest`:

```shell
cd build
ctest
```

### Running the demo app

The Amazon Chime SDK for C++ Signaling Client does not include a WebRTC implementation required for media negotiation and tranmission with Amazon Chime SDK meetings. The demo explains how to build and use [libwebrtc](https://webrtc.googlesource.com/src/) along with the client to share and receive audio and video with a Chime SDK meeting. To build and run the demo application, follow the instructions [here](demo/cli/README.md)

## Reporting a suspected vulnerability

If you discover a potential security issue in this project we ask that you notify AWS/Amazon Security via our [vulnerability reporting page](https://aws.amazon.com/security/vulnerability-reporting/). Please do not create a public GitHub issue.

## Usage

The following example is extremely simplistic and will not lead to a working example. Please use demo code as a starting point for your application, regardless of the WebRTC implementation used.

### Create a Signaling Client

To connect to a Chime SDK meeting, you need the responses from both [CreateMeeting](https://docs.aws.amazon.com/chime-sdk/latest/APIReference/API_meeting-chime_CreateMeeting.html) and [CreateAttendee](https://docs.aws.amazon.com/chime-sdk/latest/APIReference/API_meeting-chime_CreateAttendee.html). 

It is recommended to run a [serverless demo](https://github.com/aws/amazon-chime-sdk-js/blob/main/demos/serverless/README.md), since that allows web clients to join at the same time. An example script can be found in the [demo](demo/cli/run_with_serverless_demo.sh). Note that the serverless demo deploys both the browser demo *and* an AWS Lambda function that can return meeting credentials, the latter which is used by the CLI tool.

```c++
#include "signaling/default_signaling_client_factory.h"

// The following must be filled in with values retrieved from CreateMeeting and CreateAttendee
MeetingSessionCredentials credentials {
    /* attendee_id */,
    /* external_user_id */,
    /* join_token */
};
MeetingSessionURLs urls {
    /* audio_host_url */,
    /* signaling_url */
};
MeetingSessionConfiguration meeting_configuration {
    /* meeting_id */,
    /* external_meeting_id */,
    std::move(credentials),
    std::move(urls)
};

SignalingClientConfiguration signaling_client_configuration;
signaling_configuration.meeting_configuration = meeting_configuration;
DefaultSignalingDependencies signaling_dependencies {};
std::unique_ptr<SignalingClient> signaling_client = 
    DefaultSignalingClientFactory::CreateSignalingClient(
        signaling_client_configuration, std::move(signaling_dependencies));
```

#### Registering Observers

The Signaling Client requires builder code to wire up to a WebRTC implementation that is not bundled with the SDK. In the following code snippets we will reference [libwebrtc](https://webrtc.googlesource.com/src/) APIs, but usage of that library is not required. For a brief overview of peer connections on the web, see [official documentation](https://webrtc.org/getting-started/peer-connections). Most native implementations will have similar APIs so the concepts will be relevant.

The Chime SDK only supports client generated offers (i.e. the backend will never generate an offer), which simplifies the peer connection state machine and how it interacts with the Signaling Client. To receive state updates that should trigger your application code to interact with its WebRTC implementation, create a class that has access to both the Signaling Client and the WebRTC implementation, and implements SignalingClientObserver like the following:

```c++

#include "signaling/signaling_client_observer.h"

class MySignalingClientManager: public chime::SignalingClientObserver, /* May implement other observers */ {
  // Example implementations of functions will be below
}
```

Note that all functions on the observer are optional. This means you can create observers that handle lifecycle events that are seperate from observers that e.g. handle remote media state like audio mutes, etc. Any observer can be added to the Signaling Client using `AddSignalingClientObserver`

```c++
auto my_observer = std::make_unique<MySignalingClientManager>();
signaling_client_->AddSignalingClientObserver(my_observer.get());
```

#### Starting and Stopping Signaling Client

To start the signling client call `Start` and then `Run`, which will spin up a worker thread. If you would like to control the threading, call `Poll` as required instead.

```c++
signaling_client.Start();

signaling_client.Run();
// Or a naive example usage of `Poll`:
// while (true) { signaling_client.Poll() }
```

To stop the client, simply call `Stop`:

```c++
signaling_client.Stop();

```

If you implement the `OnSignalingStopped` method, you can track when all resources have been destroyed. You can also stop calling `Poll` at that point, or, if you called `Run`, you should call `StopRun`.

```c++
class MySignalingClientManager: public SignalingClientObserver {
  // ...
  virtual void OnSignalingStopped(const SignalingStatus& status) {
    // Websocket, etc. should be all destroyed by now.
    signaling_client.RemoveSignalingClientObserver(mySignalingClientManager);
    // Wait for OnSignalingStopped to be called
    signaling_client.StopRun();
  };
}
```

### Negotiating Media

The basical state machine of your application should be:

1. Observe and handle available remote video sources
2. Configure local and remote media in WebRTC implementation
3. Create and set local offer SDP in WebRTC implementation.
4. Configure the local and remote media in the Signaling Client matching WebRTC configuration
5. Set local offer SDP on the Signaling Client
6. Call `SendUpdates`
7. When the Signaling Client provides remote answer SDP via `OnRemoteDescriptionReceived`, set it on the WebRTC implementation
8. Handle media callbacks from WebRTC implmentation to start actually receiving media in your application.
9. Repeat steps 2-8 as needed in response to local client updates (e.g. enabling/disabling video) or new remote video streams.

#### Coordinating state updates between peer connections and the Signaling Client

Your application code needs to orchestrate the state of both your WebRTC implementation and the Signaling Client. The below code shows an example of how you might integrate with `libwebrtc`. Note that we use reference adapters from our demo code to allow implementation of different `libwebrtc` session description observer callbacks. The following code will not start any local/remote audio or video.

```c++
class MySignalingClientManager: public chime::SignalingClientObserver, public CreateSessionDescriptionObserver, public SetSessionDescriptionObserver, public webrtc::PeerConnectionObserver {
  // ...

  // chime::SignalingClientObserver implementations

  virtual void OnSignalingStarted(const SignalingClientStartInfo& join_info) override {
    // Connection has been established, setup new peer connection here.
  
    auto turn_credentials = join_info.credentials;
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    webrtc::PeerConnectionInterface::IceServer server;
    server.urls = turn_credentials.uris;
    server.username = turn_credentials.username;
    server.password = turn_credentials.password;
    config.servers.push_back(server);
    config.type = webrtc::PeerConnectionInterface::IceTransportsType::kRelay;
    peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);

    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    peer_connection_->CreateOffer(new rtc::RefCountedObject<MySignalingClientManager>(this), options);
  };

  virtual void OnRemoteDescriptionReceived(const std::string& sdp) override {
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description = webrtc::CreateSessionDescription (webrtc::SdpType::kAnswer, sdp, &error);
    if (!session_description) return;

    peer_connection_->SetRemoteDescription(new rtc::RefCountedObject<MySignalingClientManager>(false, this), session_description.release());
  }

  // Peer connection observer implementations
  virtual void OnCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) {
    peer_connection_->SetLocalDescription(new rtc::RefCountedObject<SetSessionDescriptionObserver>(true, this), desc);
  }

  virtual void OnCreateSessionDescriptionFailure(const webrtc::RTCError& error) {
    // Handle failure
  }

  virtual void OnSetSessionDescriptionSuccess(bool is_local) {
    if (is_local) {
      SendUpdates();
    } else {
      // Negotiation has completed!
    }
  }

  virtual void OnSetSessionDescriptionFailure(bool is_local, const webrtc::RTCError& error) {
    // Handle failure
  }


  virtual void OnIceCandidate(const IceCandidateInterface* candidate) override {
    if (!candidate) return;

    // We recommend immediately sending updates when new TURN candidates are received
    std::string url = candidate->server_url();
    if (url.find("turn") == std::string::npos) return;

    SendUpdates();
  }

  // Update helper function
  void SendUpdates() {
    // Create an offer string from peer connection
    std::string sdp;
    const webrtc::SessionDescriptionInterface* local_desc = peer_connection_->pending_local_description();
    local_desc->ToString(&sdp);

    signaling_client->SetLocalDescription(sdp);
    if (!signaling_client->SendUpdates()) {
      // Handle failure
    }
  }
}
```

#### Adding local audio and video

Once state changes are wired up, you can start by adding local audio/video using `AddOrUpdateLocalVideo`, or `AddLocalAudio`. Note that these APIs require the MIDs from created WebRTC media sections/tranceivers. If you using `libwebrtc` and are not familiar with how to create tranceivers or obtain their MIDs, see the demo for examples.

```c++
class MySignalingClientManager: /* */ {
  // ...

  virtual void SessionDescriptionObserver::OnSetSessionDescriptionSuccess(bool is_local) {
    if (is_local) {
      LocalVideoConfiguration local_video_configuration {};
      // This value helps other users decide if they want to subscribe to the video or not
      local_video_configuration.max_bitrate_kbps = 1200;
      std::string local_video_mid = // MID from SDP of builder's local video transceiver
      signaling_client.AddOrUpdateLocalVideo(local_video_mid, local_video_configuration);

      LocalAudioConfiguration local_audio_configuration {};
      local_audio_configuration.mute_state = MuteState::kMute; // or MuteState::kUnmute
      std::string local_audio_mid = // ... MID from SDP of builder's local audio transceiver
      signaling_client.AddLocalAudio(local_audio_mid, local_audio_configuration);

      // Create an offer from peer connection
      std::string sdp;
      const webrtc::SessionDescriptionInterface* local_desc = peer_connection_->pending_local_description();
      local_desc->ToString(&sdp);
      SendUpdates();
    } else {
      // Negotiation has completed!
    }
  }
}
```

### Adding Remote Video when Available

If remote video is available in the meeting, you will receive a callback on `OnRemoteVideoSourcesAvailable`. This can be wired up to create peer connection receive transceivers, which can then be used to configure remote video subscriptions via `UpdateRemoteVideoSubscriptions`.

```c++
class MySignalingClientManager /* ... */ {
  virtual void OnRemoteVideoSourcesAvailable(const std::vector<RemoteVideoSourceInfo>& sources) {
    for (const auto& source : sources) {
      // Add transceiver if we have one
    }

    // Create updated offer
    peer_connection_->CreateOffer(/* ... */);
  }

  virtual void OnRemoteVideoSourcesUnavailable(const std::vector<RemoteVideoSourceInfo>& sources) {
    for (const auto& source : sources) {
      // Remove transceiver if we have one
    }

    // Create updated offer
    controller_->peer_connection_->CreateOffer(/* ... */);
  }

  void SendUpdates() {
    if (/* New remote video transceivers have been added */) {
      // Collect all new or updated remote video subscriptions
      signaling_client->UpdateRemoteVideoSubscriptions(/* ... */)
    }

    // ... Existing code
  }
}
```

### Getting Real Time Updates

You can subscribe to the following observer callbacks for information about other remote attendees. See the demo for examples of how these callbacks are used.

```c++
class MySignalingClientManager: public SignalingClientObserver {
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

## Debugging issues in C++ SDK signaling client

### Enable Address Sanitizer Support

#### Linux

On the chime-sdk-signaling-cpp, add `-DENABLE_ASAN=ON` when generating build files.

### Enable Debug logging

You can enable debug logging by setting the log level to debug.

```c++
signaling_client->SetSignalingLogLevel("DEBUG");
```
