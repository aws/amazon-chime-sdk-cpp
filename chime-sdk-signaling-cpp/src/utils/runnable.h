// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef SIGNALING_SDK_RUNNABLE_H
#define SIGNALING_SDK_RUNNABLE_H

#include <thread>

namespace chime {
// Runnable is an abstraction that allows an implementation to inform the owning object
// of the polling model used. If polling is required, IsPollable() should be overriden and return true
// and Poll() should be overridden with polling logic.
class Runnable {
 public:
  // Implementation may require this to be called.
  // It is expected that this call is non-blocking.
  virtual void Run() {
    if (running_) return;
    running_ = true;
    run_thread_ = std::thread([=]() {
      while (running_) {
        Poll();
      }
    });
  }

  virtual void StopRun() {
    if (!running_) return;
    running_ = false;
    if (run_thread_.joinable()) run_thread_.join();
  }

  // Implementation may require this to be called in order
  // to check I/O status and dispatch events/messages.
  virtual void Poll() {}

  // Tells the owning object whether they should call Poll().
  virtual bool IsPollable() { return false; }

 private:
  std::thread run_thread_;
  bool running_ = false;
};

}  // namespace chime

#endif  // SIGNALING_SDK_RUNNABLE_H
