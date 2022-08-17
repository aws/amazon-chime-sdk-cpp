// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_VOLUME_UPDATE_H_
#define CHIME_VOLUME_UPDATE_H_

#include "utils/attendee.h"
#include "utils/volume.h"

namespace chime {

// Presents the volume and the associated attendee.
struct VolumeUpdate {
  Attendee attendee;
  Volume volume;
};

} // namespace chime

#endif //CHIME_VOLUME_UPDATE_H_
