## Migrate from 0.8.0 to 0.9.0

```cpp
// Assuming that it is using
using namespace chime;
```

### Session Dependencies
```cpp

// Before
MeetingSessionDependencies session_dependencies {};

std::unique_ptr<MeetingSession> meeting_session_ = MeetingSession::Create(session_configuration, std::move(session_dependencies));
// session_dependencies no longer accessible

// After
auto session_dependencies =
      std::make_shared<MeetingSessionDependencies>();

std::unique_ptr<MeetingSession> meeting_session_ = MeetingSession::Create(session_configuration, session_dependencies);
// session_dependencies is still accessible through shared_ptr
```

### AudioSink and AudioSource has changed to AudioNode
```cpp
// Before
class MyAudioSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

// After
class MyAudioSink final : public AudioNodeBase {
 public:
  // Almost same as OnAudioBufferReceived except it now passes as raw pointer.
  ProcessResult Process(AudioBuffer* audio_buffer) override;
}
```

### AddLocalAudioSink has changed to AddInputNode
```cpp
// Before
class MyAudioSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

auto my_audio_sink = std::make_shared<MyAudioSink>();

meeting_->audio_video->AddLocalAudioSink(my_audio_sink, Modality::kNone);


// After
class MyAudioReader final : public AudioNodeBase {
  public:
    ProcessResult Process(AudioBuffer* audio_buffer) override;
}

auto my_audio_reader = std::make_shared<MyAudioReader>();
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);

audio_context_ = dependencies->audio_context.get();
// content audio context can be retrieved from
// content_audio_context_ = dependencies->content_share_audio_context.get();

audio_context_->AddInputNode(my_audio_reader.get());
meeting_->audio_video->StartLocalAudio(Modality::kNone);
```

### RemoveLocalAudioSink has changed to RemoveInputNode
```cpp
// Before
class MyAudioSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

auto my_audio_sink = std::make_shared<MyAudioSink>();

meeting_->audio_video->RemoveLocalAudioSink(my_audio_sink, Modality::kNone);


// After
class MyAudioReader final : public AudioNodeBase {
  public:
    ProcessResult Process(AudioBuffer* audio_buffer) override;
}

auto my_audio_reader = std::make_shared<MyAudioReader>();
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);
audio_context_ = dependencies->audio_context.get();
// content audio context can be retrieved from
// content_audio_context_ = dependencies->content_share_audio_context.get();

meeting_->audio_video->StopLocalAudio(Modality::kNone);
audio_context_->RemoveInputNode(my_audio_reader.get());

```

### AddRemoteAudioSink has changed to AddOutputNode
```cpp
// Before
class MyAudioSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

auto my_audio_sink = std::make_shared<MyAudioSink>();

meeting_->audio_video->AddRemoteAudioSink(my_audio_sink);

// After
class MyAudioSink final : public AudioNodeBase {
  public:
    ProcessResult Process(AudioBuffer* audio_buffer) override;
}

auto my_audio_reader = std::make_shared<MyAudioSink>();
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);
audio_context_ = dependencies->audio_context.get();
audio_context_->AddOutputNode(my_audio_reader.get());
meeting_->audio_video->StartRemoteAudio();
```

### RemoveRemoteAudioSink has changed to RemoveOutputNode
```cpp
// Before
class MyAudioSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

auto my_audio_sink = std::make_shared<MyAudioSink>();

meeting_->audio_video->RemoveRemoteAudioSink(my_audio_sink);

// After
class MyAudioSink final : public AudioNodeBase {
  public:
    ProcessResult Process(AudioBuffer* audio_buffer) override;
}

auto my_audio_reader = std::make_shared<MyAudioSink>();
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);
audio_context_ = dependencies->audio_context.get();
audio_context_->RemoveOutputNode(my_audio_reader.get());
meeting_->audio_video->StopRemoteAudio();
```

### OnRemoteMixedAudioSourceAvailable with AddAudioSink has changed to AddOutputNode
```cpp
// Before
class MyAudioWriterSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

class ChimeSDK: public AudioVideoObserver {
  void OnRemoteMixedAudioSourceAvailable(std::shared_ptr<RemoteAudioSource> remote_audio_source) override {
    auto my_audio_writer = std::make_shared<MyAudioWriterSink>();
    remote_audio_source->AddAudioSink(my_audio_writer.get());
  }
}

// After
class MyAudioWriterNode final : public AudioNodeBase {
 public:
  ProcessResult Process(AudioBuffer* audio_buffer) override;
}
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);
audio_context_ = dependencies->audio_context.get();
auto my_audio_writer = std::make_unique<MyAudioWriterNode>();
dependencies_->audio_context->AddOutputNode(my_audio_writer.get());
meeting_->audio_video->StartRemoteAudio();
```

### OnRemoteMixedAudioSourceAvailable with RemoveAudioSink has changed to RemoveOutputNode
```cpp
// Before
class MyAudioWriterSink final : public AudioSink {
 public:
  void OnAudioBufferReceived(std::shared_ptr<AudioBuffer> audio_buffer) override;
}

auto my_audio_writer = std::make_shared<MyAudioWriterSink>();

class ChimeSDK: public AudioVideoObserver {
  void OnRemoteMixedAudioSourceAvailable(std::shared_ptr<RemoteAudioSource> remote_audio_source) override {
    remote_audio_source_ = remote_audio_source;
    remote_audio_source_->AddAudioSink(my_audio_writer.get());
  }
  private:
  std::shared_ptr<RemoteAudioSource> remote_audio_source_;
}

remote_audio_source_->RemoveAudioSink(my_audio_writer.get());

// After
class MyAudioWriterNode final : public AudioNodeBase {
 public:
  ProcessResult Process(AudioBuffer* audio_buffer) override;
}
auto dependencies =
      std::make_shared<MeetingSessionDependencies>();
auto meeting_ = MeetingSession::Create(session_configuration, dependencies);
audio_context_ = dependencies->audio_context.get();
auto my_audio_writer = std::make_unique<MyAudioWriterNode>();
dependencies_->audio_context->RemoveOutputNode(my_audio_writer.get());
meeting_->audio_video->StopRemoteAudio();
```

### Start/Stop Local audio
```cpp
// This is new API added.
// These in-turn also start/stop the underlying audio context input.

// StartLocalAudio(modality)
// meeting_ = MeetingSession::Create(session_configuration, dependencies);
meeting_->audio_video->StartLocalAudio(Modality::kNone);

// StopLocalAudio(modality)
meeting_->audio_video->StopLocalAudio(Modality::kNone);
```

### Receiving/Stopping Remote audio
```cpp
// This is new API added.
// These in-turn also start/stop the underlying audio context output

// Receive remote audio
// meeting_ = MeetingSession::Create(session_configuration, dependencies);
meeting_->audio_video->StartRemoteAudio();

// Stop receive remote audio
meeting_->audio_video->StopRemoteAudio();
```
