# API Overview

This guide gives an overview of the API methods that you can use to create a meeting with audio, video, and data messages. You can implement all functionality or just a subset. The examples in this overview may not provide every possible configuration and use case, but hopefully provide a starting point for learning how to use these APIs.

## 1. Create a session

The `MeetingSession` and its `AudioVideoController` are the starting points for creating meetings. Like many classes, the `MeetingSession` take a configuration struct and a dependencies struct. We will show how these are created and used when creating the `MeetingSession`. We'll also show how to inject a logger.

### **1a. Create a logger**

You can utilize the `ConsoleLogger` to write to [std::cout](https://en.cppreference.com/w/cpp/io/cout) or implement the `Logger` interface to customize the logging behavior.

```
std::unique_ptr<chime::Logger> logger = std::make_unique<chime::ConsoleLogger>(chime::LogLevel::kDebug);
```
The log levels available are listed in `utils/logger/log_level.h`.

### **1b. Create a meeting session configuration**

Create a `MeetingSessionConfiguration` with the responses to [chime:CreateMeeting](https://docs.aws.amazon.com/chime/latest/APIReference/API_CreateMeeting.html) and [chime:CreateAttendee](https://docs.aws.amazon.com/chime/latest/APIReference/API_CreateAttendee.html). Your server application should make those API calls and securely pass the meeting and attendee responses to the client application.

```
chime::MeetingSessionCredentials credentials{attendee_id,
                                             external_user_id,
                                             join_token};

chime::MeetingSessionURLs urls{audio_host_url,
                               signaling_url};

chime::MeetingSessionConfiguration configuration{meeting_id,
                                                 external_meeting_id,
                                                 std::move(credentials),
                                                 std::move(urls)};
        
```

### **1C. Create Meeting dependencies**

Create `MeetingSessionDependencies` to pass dependencies. All dependencies are optional with defaults set internally when they are not set.

```
chime::MeetingSessionDependencies dependencies{};
dependencies.logger = std::move(logger);
```

### **1D. Create the meeting session**

Create a `MeetingSession` with the `MeetingSessionDependencies` and `MeetingSessionConfiguration`.

```
std::unique_ptr<chime::MeetingSession> meeting_session =
    MeetingSession::Create(std::move(configuration), std::move(dependencies));
```

## 2. Register an audio video observer

You can receive events about the state of the meeting session and remote video sources by implementing an `AudioVideoObserver` and registering the observer with the `AudioVideoController`. A default implementation of the `AudioVideoController` called `DefaultAudioVideoController` is created when calling `MeetingSession::Create`. The `AudiovideoController` is an attribute of the `MeetingSession`. The following is an example implementation of an `AudioVideoObserver`. You can implement as many or as few observer methods as you like.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
 public:
  chime::Logger logger;
  
  void OnAudioVideoSessionConnecting(
      chime::MeetingSessionStatus session_status) override {
    logger.Info("Session connecting");
  }

  void OnAudioVideoSessionStarted(
      chime::MeetingSessionStatus session_status) override {
    logger.Info("Session started");
  }

  void OnAudioVideoSessionStopped(
      chime::MeetingSessionStatus session_status) override {
    logger.Info("Session stopped with status: " +
                std::to_string(static_cast<int>(session_status.status_code)));
  }
  
  void OnRemoteVideoSourcesAvailable(
    std::vector<std::shared_ptr<chime::RemoteVideoSource>> remote_video_sources
  ) {
    ... 
    
    // Add a VideoSink to each RemoteVideoSource
    // You may want to keep track of the video sinks that were added for removal purposes later
    remote_video_source->AddVideoSink(video_sink);
  }
};
```

To add an `AudioVideoObserver`, call `AudioVideoController::AddAudioVideoObserver(observer)`.

```
MyAudioVideoObserver* my_audio_video_observer = /* pointer to an instance of MyAudioVideoObserver */;

meeting_session->audio_video->AddAudioVideoObserver(my_audio_video_observer);
```

To remove the `AudioVideoObserver` call `AudioVideoController::RemoveAudioVideoObserver(observer)`.

```
meeting_session->audio_video->RemoveAudioVideoObserver(my_audio_video_observer);
```

An `AudioVideoObserver` has the following functions related to meeting session:

* `OnAudioVideoSessionConnecting`: Called when the meeting session is connecting 
* `OnAudioVideoSessionStarted`: Called when the meeting session has connected
* `OnAudioVideoSessionStopped`: Called when meeting session has disconnected

## 3. Start and stop the meeting session

Call this method after doing pre-requisite configuration (See previous sections).

To start the meeting session, call `AudioVideoController::Start`. This will start the underlying media client.

```
meeting_session->audio_video->Start();
```

To stop the meeting session, call `AudioVideoController::Stop`.

```
meeting_session->audio_video->Stop();
```

## 4. Send and receive video and/or content

You can use the following methods in order to send and receive video and/or content.

`VideoSource` is an interface for sources which produce video and/or content frames. You can send the frames to a `VideoSink` that consumes video and/or content frames which may process, fork, or render these frames. You can assign `VideoContentHint` to your video source to inform the content type, which mimics [WebRTC video content hints](https://www.w3.org/TR/mst-content-hint/#video-content-hints).

### **4a. Sending video and/or content**

To send your video and/or content to other meeting attendees, create a `VideoSource` and call `AudioVideoController::AddLocalVideo`. You will need to specify `Modality::kNone` for video and `Modality::kContent` for content. Currently there is limitation on 1 video and 1 content share video.

```
class MyVideoSource : public chime::VideoSource {
 public:
  // This source emphasizes motion over detail
  chime::VideoContentHint video_contentHint = chime::VideoContentHint::kMotion;
  
  void AddVideoSink(chime::VideoSink* sink) override {
    local_preview = sink;
  }
  
  void RemoveVideoSink(chime::VideoSink *sink) override {
    local_preview = nullptr;
  }
  
 private:
  // Local video sink for preview, could also use a collection of sinks
  chime::VideoSink* local_preview; 
  
  // Sample function to start generating video frames
  void StartGeneratingFrames() {
    // ...
    
    chime::VideoFrame* frame = /* generated video frame */;
    local_preview->OnVideoFrameReceived(frame);
    
    // ...
  }
  
  // ...
}

MyVideoSource* my_video_source = /* pointer to the instance of MyVideoSource */;
my_video_source->StartGenerateFrames();
meeting_session->audio_video->AddLocalVideo(my_video_source, Modality::kNone);
meeting_session->audio_video->AddLocalVideo(my_content_source, Modality::kContent);
```

To stop sending video and/or content, call `AudioVideoController::RemoveLocalVideo`.

```
meeting_session->audio_video->RemoveLocalVideo(Modality::kNone);
meeting_session->audio_video->RemoveLocalVideo(Modality::kContent);
```

### **4b. Receiving video**

To receive video from other meeting attendees, first create a `VideoSink` to process received video frames from a `VideoSource`.


```
class MyVideoSink : public chime::VideoSink {
 public:
   void OnVideoFrameReceived(std::shared_ptr<chime::VideoFrame> frame) override {
     // Process, fork or render video frame
   }
}
```

The `AudioVideoObserver` has the following functions related to receiving remote video. These functions provide a vector of `RemoteVideoSource`s that you can call `VideoSource::AddVideoSink` with.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
  ...
  
  // Invoked when new remote video sources are available
  void OnRemoteVideoSourcesAvailable(
    std::vector<std::shared_ptr<chime::RemoteVideoSource>> remote_video_sources
  ) {
    ... 
    
    // Add a VideoSink to each RemoteVideoSource
    // You may want to keep track of the video sinks that were added for removal purposes later
    remote_video_source->AddVideoSink(video_sink);
  }
 
  // Invoked when remote video sources are no longer available.
  // Only invoked when a remote attendee stops sending their video.
  void OnRemoteVideoSourcesUnavailable(
    std::vector<std::shared_ptr<chime::RemoteVideoSource>> remote_video_sources) {
    ...
    
    // Remove any VideoSink(s) that were added to each RemoteVideoSource
    remote_video_source->RemoveVideoSink(video_sink);
  }
}
```

## 5. Send and receive audio

You can use the following to send and receive audio. Currently, all the audio is mixed into one stream that is always available for receiving.

`AudioSource` is an interface for sources which produce audio samples, and can send to an `AudioSink` that consumes audio samples and may process, fork, or playback these samples. Note that the `AudioSource` needs to send 10ms of data at a time. This is the currently supported configuration for `AudioBuffer`:

* Sample format - 16 signed PCM
* Sample rate - 48khz
* Number of channels -
  * Speaker: Mono and Stereo
  * Mic: Mono

Typically, `AudioSource` is implemented by classes that are used by the `AudioDriver`. A custom `AudioDriver` can be injected, but internal system specific drivers will be used by default.

### **5a. Sending audio**

To overwrite audio sent from the audio driver (the audio driver typically sends mic audio), optionally create an `AudioSink` and call `AudioVideoController::AddLocalAudioSink` passing in `Modality::kNone`. To send content audio, it is necessary to call `AudioVideoController::AddLocalAudioSink` passing in `Modality::kContent`. Content audio will not be sent otherwise.

```
class MyAudioSink : public chime::AudioSink {
 public:
   void OnAudioBufferReceived(std::shared_ptr<chime::AudioBuffer> audio_buffer) override {

     // Use AudioBuffer metadata to determine size of the sample.
     audio_buffer.data = // overwrite with 10ms of audio.
   }
}

// audio
MyAudioSink* my_audio_sink = /* pointer to the instance of MyAudioSink */;
meeting_session->audio_video->AddLocalAudio(my_audio_sink, Modality::kNone);

// content
MyAudioSink* my_audio_content_sink = /* pointer to the instance of MyAudioSink */;
meeting_session->audio_video->AddLocalAudio(my_audio_content_sink, Modality::kContent);
```

### 5B. Receiving Audio

To access the meeting's mixed audio stream before the audio driver processes it for playback, implement `AudioVideoObserver::OnRemoteMixedAudioSourceAvailable` and bind your `AudioSink` implementation to the mixed `RemoteAudioSource` by calling `RemoteAudioSource::AddAudioSink`.

```
class MyAudioSink : public chime::AudioSink {
 public:
   void OnAudioBufferReceived(std::shared_ptr<chime::AudioBuffer> audio_buffer) override {
     // Process, fork or play audio samples
   }
}

MyAudioSink* my_audio_sink_ = /* pointer to the instant of MyAudioSink */;

// ...

class MyAudioVideoObserver : public chime::AudioVideoObserver {
 public:
  void OnRemoteMixedAudioSourceAvailable(
    std::shared_ptr<chime::RemoteAudioSource> remote_audio_source) {
    // Binding audio sink
    remote_audio_source->AddAudioSink(my_audio_sink_);
  }
};

```

To stop intercepting audio, call `RemoteAudioSource::RemoveAudioSink`. This will not stop the `AudioDriver` from sending to a playback device (usually speakers).

```
remote_audio_source->RemoveAudioSink(my_audio_sink_);
```

To mute or unmute local audio or content audio call `SetLocalAudioMute` on the `AudioVideoController` with the respective modality and mute state desired.

```
// This unmutes content audio
meeting_->audio_video->SetLocalAudioMute(false, Modality::kContent);
```

## 6. Build a roster of participants

### 6A. Registering for roster events

You can use `AudioVideoObserver` to also get callbacks when attendees join and leave and when their volume level, mute state, or signal strength changes. 

You can implement the following methods in the same `AudioVideoObserver` for meeting lifecycle events or create another class and register it by calling `AudioVideoController::AddAudioVideoObserver(observer)`.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
  ...
  void OnAttendeeJoined(const chime::Attendee& attendee) {
    logger.Info("Attendee joined the meeting: " + attendee.DebugString());
  }
 
  void OnAttendeeLeft(const chime::Attendee& attendee) {
    logger.Info("Attendee left the meeting: " + attendee.DebugString());
  }
 
  void OnAttendeeDropped(const chime::Attendee& attendee) {
    logger.Info("Attendee dropped the meeting: " + attendee.DebugString());
  }
 
  void OnAttendeeAudioMuted(const chime::Attendee& attendee) {
    logger.Info("Attendee muted: " + attendee.DebugString());
  }
 
  void OnAttendeeAudioUnmuted(const chime::Attendee& attendee) {
    logger.Info("Attendee unmuted: " + attendee.DebugString());
  }
 
  void OnVolumeUpdates(const std::vector<chime::VolumeUpdate>& updates) {
    for (chime::VolumeUpdate update : updates) {
      logger.Info("Volume updated: volume: " +
        std::to_string(update.volume.normalized_volume) +
        ", attendee: " + update.attendee.DebugString());
    }
  }
 
  void OnSignalStrengthUpdates(const std::vector<chime::SignalStrengthUpdate>& updates) {
    for (chime::SignalStrengthUpdate update : updates) {
      logger.Info("Signal strength updated: signal: " +
        std::to_string(update.signal_strength.normalized_signal_strength) +
        ", attendee: " + update.attendee.DebugString());
    }
  }
  ...
};
```

AudioVideoObserver has the following methods that related to roster events:

* `OnAttendeeJoined`: Called when an attendee has joined the meeting.  A newly joined attendee will receive this callback for attendees that are currently in the meeting.
* `OnAttendeeLeft`: Called when an attendee has left the meeting.
* `OnAttendeeDropped`: Called when an attendee has dropped from the meeting.
* `OnAttendeeAudioMuted`: Called when an attendee's audio has muted.
* `OnAttendeeAudioUnmuted`: Called when an attendee's audio has unmuted.
* `OnVolumeUpdates`: Called when the updated volumes are available. Only contains volumes that changed.
* `OnSignalStrengthUpdates`: Called when the updated signal strengths are available. Only contains signal strength that changed.

All callbacks provide both the attendee ID and external user ID from [chime:CreateAttendee](https://docs.aws.amazon.com/chime/latest/APIReference/API_CreateAttendee.html) so that you may map between the two IDs.

## 7. Send and receive data message

Data message allows you send small payload (max 2KB) of any message in a real time. Data messages can be used to signal attendees of changes to meeting state or develop custom collaborative features. Each message is sent on a particular topic, which allows you to tag messages according to their function to make it easier to handle messages of different types.

### **7a. Sending Data Message**

When sending a message if you specify a lifetime, then the media server stores the messages for the lifetime. Up to 1024 messages may be stored for a maximum of 5 minutes. Any attendee joining late or reconnecting will automatically receive the messages in this buffer once they connect. You can use this feature to help paper over gaps in connectivity or give attendees some context into messages that were recently received.

To send data message to given topic, call `SendDataMessage` API.

```
chime::DataMessageToSend data_message_to_send {};
data_message_to_send.topic = "chat"; // topic to send
data_message_to_send.data = "Hello World"; // data to send
data_message_to_send.lifetime_ms = 300000; // max five minutes
meeting_->audio_video->SendDataMessage(data_message_to_send);
```

### **7B. Receiving Data Message**

To receive messages, register an observer as described in [2. Register an audio video observer](#2-register-an-audio-video-observer). In the callback, you’ll receive `DataMessageReceived` that contains information about payload of the message and other metadata about the message.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
 public:
  chime::Logger logger;

  void OnDataMessagesReceived(
    const std::vector<chime::DataMessageReceived>& messages) {
    for (const auto& message: messages) {
      logger.Info("Received: " + message.data + " sent by " + message.attendee.external_user_id + " at " + std::to_string(message.timestamp_ms));
    }
  }
}
```

### **7C. Receiving Data Message That Failed To Send**

If you send too many messages at once, your messages may be returned to you through `OnDataMessagesFailedToSend` API with `DataMessageSendErrorReason` of *`kThrottled.`*  The current throttling soft limit for Data Messages is 100 messages per second with the maximum burst size of 200 for a meeting (i.e. a 'token bucket' of size 200 that refills at 100 tokens per second). If you continue to exceed the throttle limit, then the server may hang up the connection. The hard limit for each attendee is 200 messages per second with the maximum burst of 2000 and for a meeting is 500 messages per second with the maximum burst of 10000.

See the following code snippet for handling `OnDataMessagesFailedToSend`.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
 public:
  chime::Logger logger;
  
  void OnDataMessagesFailedToSend(
    const std::vector<chime::DataMessageSendError>& to_send_errors) {
    for (const auto& error: to_send_errors) {
      logger.Error("Data message " + error.data_message.data + " failed to send: " +  + "with error " + std::to_string(static_cast<int>(error.reason)));
    }
  }  
}
```

### 7D. Dispatch Data message by topic

You can have one single `AudioVideoObserver` to receive messages from all topics and handle them correspondingly inside the class, or you can use `DataMessageDispatcher` as a helper class to be able to register multiple classes that expect to handle the messages from a certain topic.

```
// Create the dispatcher. When data messages are received, the dispatcher filters them by the string `DataMessage::topic` provided by the sender of the message.
auto dispatcher = std::make_unique<DataMessageDispatcher>();
```

Then implement your data message handlers. For example, you can implement a chat message handler.
```
class MyChatMessagesHandler : public AudioVideoObserver {
 public:
  void OnDataMessagesReceived(const std::vector<DataMessageReceived>& messages) override {
    
    // Handle data message. For example, you could print to the console.
    for (auto message : messages) {
      std::cout << message.data << std::endl;
    }

  }

  void OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) {

    // Handle data message send failure. For example, you could print to the console.
    for (auto error : to_send_errors) {
      std::cout << "Failed to send data message. Error: " + std::to_string(static_cast<int>(error.reason)) << std::endl;
    }

  }
};
```

You could also implement a whiteboard message handler to post messages to a virtual whiteboard.
```
class MyWhiteboardMessagesHandler : public AudioVideoObserver {
 public:
  void OnDataMessagesReceived(const std::vector<DataMessageReceived>& messages) override {

    for (auto message : messages) {
      // Post message to virtual whiteboard
    }

  }

  void OnDataMessagesFailedToSend(const std::vector<DataMessageSendError>& to_send_errors) {
    
    for (auto error : to_send_errors) {
      std::cout << "Failed to post message to whiteboard. Error: " + std::to_string(static_cast<int>(error.reason)) << std::endl;
    }

  }
};
```

Then pass your custom data message handlers into `DataMessageDispatcher::SubscribeToTopic` with a relevant topic string.

For each message, the dispatcher will distribute the message to each observer that is subscribed to its topic.
```
//  `MyChatMessagesHandler` expects messages from `chat` topic
auto chat_handler = std::make_unique<MyChatMessagesHandler>();
// `MyWhiteBoardMessagesHandler` expects messages from `whiteboard` topic
auto whiteboard_handler = std::make_unique<MyWhiteBoardMessagesHandler>();
dispatcher->SubscribeToTopic(chat_handler.get(), "chat");
dispatcher->SubscribeToTopic(whiteboard_handler.get(), "whiteboard");
// Multiple observers can subscribe to the same topic.
// For example, suppose the virtual whiteboard should also display `chat` messages in addition to `whiteboard` messages.
dispatcher->SubscribeToTopic(whiteboard_handler.get(), "chat");
// Add dispacther to the meeting session
meeting_session->audio_video->AddAudioVideoObserver(dispatcher.get());
```

You should call corresponding `UnsubscribeFromTopic` after the meeting stop or whenever you need to.

This will prevent a specified observer from receiving messages with a particular topic.

```
// For example, a trigger causes the virtual whiteboard to no longer receive `chat` messages, but still receive `whiteboard` messages.
dispatcher->UnsubscribeFromTopic(whiteboard_handler.get(), "chat");
```

## 8. Proxy Support

Sometimes you might want to add proxy to the turn url. In order to apply the changes in turn urls, implement `TurnUrlRewriter` and pass to `MeetingSessionConfiguration` as `shared_ptr.` If you do not pass anything, it will be no-op. 

>NOTE: Unlike other SDKs, this only modifies the turn url. Other url such as signaling url should be modified by the builders and then passed to SDK layer through MeetingSessionConfiguration

See the below code.

```
class NoOpTurnUrlRewriter : public chime::TurnUrlRewriter {
 public:
  std::string RewriteTurnUrl(const std::string& url) override {
    return url;
  }
};
```

```
dependencies->turn_url_rewriter = std::make_unique<NoOpTurnUrlRewriter>();

// Now you can create meeting session.
```

>NOTE: You shouldn’t proxy the audio_host_url.

Here is an example of rewriting signaling url

```
// assumes there is utils class that replace by given string or regex
auto ws_uri = utils.replace(uri, "wss", "https");
auto replaced_uri = utils.replace(uri, "\.*signal.*?aws/gi", "proxy.mydomain.com") + utils.EncodeUrl(ws_uri);
return replaced_uri;
```

## 9. Receive media metrics

You can receive media metrics of the current meeting session by implementing `OnMediaMetrics` in `AudioVideoObserver`. The metrics will be delivered after the meeting starts, and then on a one second interval until meeting stops. You can call the provided `DebugString` method to print out the metrics as a string or process the metrics however you’d like.

```
class MyAudioVideoObserver : public chime::AudioVideoObserver {
 public:
  chime::Logger logger;
  
  void OnMediaMetrics(const chime::MediaMetrics& metrics) {
    logger.Debug("Received media metrics: " + metrics.DebugString()); 
}
```

## 10. Receive updates for active speakers

You can receive a list of active speakers in descending order of volume score whenever there are updates on the list by implementing `OnActiveSpeakersDetected` in ActiveSpeakerObserver. The list will be delivered whenever there is an update of active speakers.

```
class ActiveSpeakerObserver {
public:   
    virtual void OnActiveSpeakersDetected(const std::vector<Attendee>& attendees) = 0;
}
```

Here is an example how you can use this feature.

1. Create a class which inherits `ActiveSpeakerObserver` and implements the logic for the callback function `OnActiveSpeakersDetected` to get the update for active speakers.

```
class MyActiveSpeakerObserver : public chime::ActiveSpeakerObserver {
 public:
  void OnActiveSpeakersDetected(const std::vector<Attendee>& attendees) {
    std::string active_speaker_list = "";
    for (Attendee attendee : attendees) {
      active_speaker_list += attendee.external_user_id + " ";
    }
    std::cout << "Active speaker list is updated as: " << active_speaker_list << std::endl;
  }
};
```

```
auto my_active_speaker_observer = std::make_unique<MyActiveSpeakerObserver>();
```

1. Create an `ActiveSpeakerDetector` passing the class created in step 1 to `ActiveSpeakerDetector` using the Create function in `ActiveSpeakerDetector`.

```
 std::unique_ptr<ActiveSpeakerDetector> active_speaker_detector =
      chime::ActiveSpeakerDetector::Create(my_active_speaker_observer.get());
```

1. Register `ActiveSpeakerDetector` with meeting_session→audio_video→AddAudioVideoObserver. The active speaker feature will be enabled.

```
  meeting_session->audio_video->AddAudioVideoObserver(active_speaker_detector.get());
```

## 11. Receive transcription events

Transcription can be used to enable transcription of the top two active talkers. See [the live transcription guide](https://docs.aws.amazon.com/chime/latest/dg/meeting-transcription.html) to create necessary service-linked role so that the other demo applications such as JS, Android, or iOS can call Amazon Transcribe and Amazon Transcribe Medical on your behalf. See [data detail](https://docs.aws.amazon.com/chime/latest/dg/process-msgs.html) to know further about how to process the data, the following sample only prints out the basic values.

```
class MyTranscriptionObserver : public chime::TranscriptEventObserver {
 public:
  chime::Logger logger;
 
  void OnTranscriptionStatusReceived(const chime::TranscriptionStatus& status) override {
    logger_->Info("Live Transcription " + chime::TranscriptionStatus::GetTypeName(status.type) +
            " at " + std::to_string(status.event_time_ms) +
            " in " + status.transcription_region +
            " with configuration: " + status.transcription_configuration);
  }
  
  void OnTranscriptReceived(const chime::Transcript& transcript) override {
    logger_->Debug("Receiving transcript: " + transcript.DebugString());
    for (const chime::TranscriptResult& result : transcript.results) {
      if (result.alternatives.empty()) continue;
      
      auto alternative = result.alternatives[0];
      std::string name = "<Unknown>";
      
      if (!alternative.items.empty()) {
        name = alternative.items[0].attendee.external_user_id;
      }
      
      logger_->Info("Transcript result [" + result.result_id + "] " + name + ": " + alternative.transcript);
    }
};

auto transcription_handler = std::make_unique<MyTranscriptionObserver>();
auto dispatcher = std::make_unique<chime::DataMessageDispatcher>();
auto transcription_controller =
      chime::TranscriptionController::Create(transcription_handler.get());
dispatcher->SubscribeToTopic(transcription_controller.get(), chime::kTranscriptionDataMessageTopic);
meeting_session->audio_video->AddAudioVideoObserver(dispatcher.get());
```

## 12. Inject a custom audio driver

By default, an internal system dependent audio driver is used. However, you have the option to inject your own.

Some reasons you might want to override the default audio driver:

 - You want to use a third party library to access devices.
 - You want direct control over the interactions between the audio stream and the audio devices used.

First implement your `AudioDriver`

```
class MyAudioDriver: public AudioDriver {
 public:
  int Init(
      const AudioDriverConfiguration& playback_config,
      std::unique_ptr<PlaybackCallback> playback_callback,
      const AudioDriverConfiguration& record_config,
      std::unique_ptr<RecordCallback> record_callback) override {
   
     // Validate configurations.
     
     // Do any initialization of system library.
   
   }

  int Start() override {
    // Call system APIs to start playback and/or recording.
  }

  int Stop() override {
    // Stop system playback and/or recording.
  }

  void Destroy() override {
    // Cleanup of system device management.
  }

  AudioDriverConfiguration PlaybackConfiguration() const override {
    // return the playback configuration.
  }
  AudioDriverConfiguration RecordConfiguration() const override {
    // return the record configuration.
  }

  std::string GetImplementationId() const override {
    // return an id to identify this audio driver implementation.
  }
  ...
};
```

You can also use the `AudioDeviceEnumerator` to register a device change observer with your system API.

```
class MyAudioDeviceEnumerator: public AudioDeviceEnumerator {
 public:
  std::vector<AudioDeviceInfo> EnumerateAudioDevices() override {
    // Use system API to check the available devices.
    // You can store them or use the system API to choose a device.
  }

  int RegisterAudioDeviceChangeObserver(
      std::unique_ptr<AudioDeviceChangeObserver> device_change) override {
    observer_ = std::move(device_change);
    // Register the below callback function with system API.
  }
  
  // Implement a system API specific callback. The signature will differ depending on the API.
  static int callback(std::vector<int> device_ids, void* ctx) {
    auto self = static_cast<MyAudioDeviceEnumerator*>(ctx);
    auto devices = self->EnumerateAudioDevices();
    // Use system API to choose a device from devices.
    self->observer_->AudioDeviceListDidChange();
  }
 private:
  std::unique_ptr<AudioDeviceChangeObserver> observer_;
}

class MyAudioDeviceChangeObserver: public AudioDeviceChangeObserver {
 public:
  void AudioDeviceListDidChange() override {
    // Additional handling of device change.
  }
}
```

Then instantiate and inject your audio driver.

```
MeetingSessionDependencies my_meeting_session_dependencies;
...
my_meeting_session_dependencies.custom_audio_driver = std::make_unique<chime::AudioDriver>(MyAudioDriver());

MeetingSessionConfiguration my_meeting_session_configuration;
...
my_meeting_session_configuration.audio_video_configuration.audio_configuration.playback_configuration = // configure playback configuration
my_meeting_session_configuration.audio_video_configuration.audio_configuration.record_configuration = // configure record configuration

// Create a meeting session as described in the first section.
```

