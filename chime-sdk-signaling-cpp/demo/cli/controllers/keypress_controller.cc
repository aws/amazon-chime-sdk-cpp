// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "keypress_controller.h"

#include <iostream>

KeypressController::KeypressController(std::shared_ptr<MeetingController> controller) : controller_(std::move(controller)) {}

int KeypressController::Exec() {
  std::cout << "Starting keypress handler" << std::endl;

  char c;
  do {
    std::cin >> c;
    OnKeypress(c);
  } while (c != 'q');

  return 0;
}

void KeypressController::OnKeypress(char key) {
  std::cout << "Keypress received: " << std::string(1, key) << std::endl;
  if (!controller_) return;

  switch (key) {
    case 'q':  // Quit
      controller_->Stop();
      break;
    case 'c':  // Toggle capturer start/stop
      controller_->StartLocalVideo();
      break;
    case 'd':  // Send data message
      controller_->SendDataMessage("Hello from Signaling SDK demo!");
    default:
      break;
  }
}
