// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNAL_STRENGTH_UPDATE_H_
#define CHIME_SIGNAL_STRENGTH_UPDATE_H_

#include "utils/attendee.h"
#include "utils/signal_strength.h"

namespace chime {

// Presents the signal strength and the associated attendee.
struct SignalStrengthUpdate {
  Attendee attendee;
  SignalStrength signal_strength;
};

} // namespace chime

#endif //CHIME_SIGNAL_STRENGTH_UPDATE_H_
