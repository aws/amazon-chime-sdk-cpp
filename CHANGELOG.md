# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.8.0] - 2023-02-03
### Added
* Added `size_y_`, `size_u_`, and `size_v_` to `I420VideoFrameBuffer`

### Fixed
* (**Breaking**) Fixed incorrect values for `I420VideoFrameBuffer`. Previously, values for `size_y_`, `size_u_`, and `size_v_` were used as the values for `stride_y_`, `stride_u_`, and `stride_v_`. Anything relying on size values being used for strides now needs to calculate size or use the newly provided fields
* Fixed `width_` and `height_` values missing for `I420VideoFrameBuffer`

## [0.7.0] - 2022-09-16

### Added
* On `AudioVideoController`, added `SetLocalAudioMute`
* On `AudioVideoController`, added `AddRemoteAudioSink` and `RemoveRemoteAudioSink`. These are additional ways to add audio sinks. One can still add audio sinks with `OnRemoteAudioSourceAvailable`.
* On `AudioVideoObserver`, added `OnLocalAudioSourceAvailable`
* `CVPixelVideoFrameBuffer` used for iOS and OSX specific `VideoFrame`s
* `AudioDriver` and associated classes/structs in `audio_driver.h`. Internally system specific drivers will be used unless custom driver is injected.
* Custom `AudioDriver` added to `MeetingSessionDependencies` in order to allow `AudioDriver`s to be injected
* `kFloat32` added as additional audio sample format type for `SampleFormat`
* `PullAudio` method added to `RemoteAudioSource`
* `LocalAudioSource`, an implementation of `AudioSource`

### Changed
* (**Breaking**) On `AudioVideoController`, `AddLocalAudioSink` replaces `AddLocalAudio` and `Modality` is passed in directly. `AudioSink` replaces `AudioSource` as a parameter
* (**Breaking**) On `AudioVideoController`, `RemoveLocalAudioSink` replaces `RemoveLocalAudio`
* (**Breaking**) On `AudioVideoObserver`, `OnLocalAudioSourceAvailable` replaces `OnLocalAudioSourceBecameAvailable`
* (**Breaking**) On `AudioVideoObserver`, `OnAudioVideoSessionConnecting` replaces `AudioVideoSessionDidStartConnecting`
* (**Breaking**) On `AudioVideoObserver`, `OnAudioVideoSessionStarted` replaces `AudioVideoSessionDidStart`
* (**Breaking**) On `AudioVideoObserver`, `OnAudioVideoSessionStopped` replaces `AudioVideoSessionDidStop`
* (**Breaking**) On `AudioVideoObserver`, `OnRemoteAudioSourceBecameAvailable` replaces `OnRemoteMixedAudioSourceAvailable`

### Removed
* (**Breaking**) Removed `AudioSourceConfiguration`
* Removed usage of Unified Plan video client flag which no longer exists.

## [0.6.0] - 2022-02-09

### Added
* Added `DataMessageDispatcher` to help dispatch messages by topic.
* Added support for integration with Amazon Transcribe and Amazon Transcribe Medical for live transcription. The Amazon Chime Service 
uses its active talker algorithm to select the top two active talkers, and sends their audio to Amazon Transcribe 
(or Amazon Transcribe Medical) in your AWS account. User-attributed transcriptions are then sent directly to every meeting 
attendee via data messages. Use transcriptions to overlay subtitles, build a transcript, or perform real-time content analysis. 
For more information, visit [the live transcription guide](https://docs.aws.amazon.com/chime/latest/dg/meeting-transcription.html).
* Added `RemoteVideoSource` as part of a replacement for removing `BindVideoSink` and `UnbindVideoSink` from `AudioVideoController`. You can call `RemoteVideoSource`'s `AddVideoSink` and `RemoveVideoSink` instead of `AudioVideoController`'s `BindVideoSink` and `UnbindVideoSink`.
* Added `OnRemoteVideoSourceUnavailable` callback to `AudioVideoObserver`.
* Added support for receiving stereo audio.

### Changed
* (**Breaking**) Changed `OnRemoteVideoSourceAvailable` to `OnRemoteVideoSourcesAvailable`. Also changed the parameter from a shared pointer to a `RemoteVideoSource` to a vector of shared pointers to `RemoteVideoSource`. This is part of the changes to replace `BindVideoSink` and `UnbindVideoSink`.

### Removed
* (**Breaking**) Removed `BindVideoSink` and `UnbindVideoSink` from `AudioVideoController`. See replacement `RemoteVideoSource` and changes to `OnRemoteVideoSourceAvailable` for details on how the replacement works

## [0.5.0] - 2021-10-01

### Added
* Add media metrics functionality.
* Added active speaker functionality.

### Changed
* [**Breaking**] Added MeetingSessionDependencies
  * [**Breaking**] MeetingSessionConfiguration does not take MeetingSessionUrls and MeetingSessionCredentials as unique_ptr, but instead normal struct
  * [**Breaking**] Logger is no longer exposed from `MeetingSessionConfiguration`
  * [**Breaking**] `Create` function in `MeetingSession` and `DefaultAudioVideoController` now takes `MeetingSessionDependencies` and `MeetingSessionConfiguration` as value.
  * [**Breaking**] MeetingSession no longer takes `MeetingSessionConfiguration` as pointer.

## [0.4.0] - 2021-09-10

### Added
* Add turn url rewriter
* Added data message send and receive functionality.
* Added roster events functionality.

### Changed
* (**Breaking**) Renamed enum, SampleFormat::kSigned16 to SampleFormat::kInt16.
* (**Breaking**) Renamed function from ToString to DebugString for MeetingSessionConfiguration, MeetingSessionURLs and MeetingSessionCredentials.
* (**Breaking**) Renamed struct AttendeeInfo to Attendee.

### Fixed
* Fixed the case when attendee wasn't present if audio source has not been added and was not sending.

## [0.3.0] - 2021-08-13

### Added
* Added windows build with CMake.
* Added audio receive functionality.
* Added audio send functionality. 

### Fixed
* Fixed empty frames sending when no content source added.

## [0.2.0] - 2021-07-16

### Added
* [Documentation] Added comments.
* Added content share functionality.

## [0.1.0] - 2021-05-28

### Added
* Added meeting session creation components.
* Added logger functionality.
* Added meeting lifecycle functionality.
* Added video send and receive functionality.

### Fixed
* Fixed video frame timestamps when used causing no video sent beyond first frame.
* Deduplicated OnRemoteVideoSourceAvailable and AudioVideoSessionDidStart callbacks.
