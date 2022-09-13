# Unreleased

## Fixed
* Fixed stop behavior to end gracefully when stop is called.
* Fixed handling of fragmented messages on libwebsockets

## Changed
* **Breaking** `Stop()` on `WebsocketSignalingTransport` no longer calls `StopRun`
* `Runnable` now uses `atomic` to be thread-safe

# 0.1.0 - 2022-08-19

## Added
* [Demo] Added support for sending audio using file.
* [Demo] Added support for receiving audio through file.
* [Demo] Added support for sending video through custom source that creates black and green frame. 
* [Demo] Added support for receiving video through file.