// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_KEYPRESS_CONTROLLER_H_
#define CHIME_KEYPRESS_CONTROLLER_H_

#include <map>

#include "controllers/meeting_controller.h"

class KeypressController final {
 public:
  KeypressController(std::shared_ptr<MeetingController> meeting_controller);

  int Exec();

 private:
  void OnKeypress(char key);

  std::shared_ptr<MeetingController> controller_;
};

#endif  // CHIME_KEYPRESS_CONTROLLER_H_
