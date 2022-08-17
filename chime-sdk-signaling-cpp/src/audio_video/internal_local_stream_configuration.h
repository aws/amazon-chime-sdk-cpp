//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_INTERNAL_LOCAL_STREAM_CONFIGURATION_H_
#define CHIME_SIGNALING_INTERNAL_LOCAL_STREAM_CONFIGURATION_H_

#include "utils/attendee.h"

namespace chime {
struct InternalStreamConfiguration {
  Attendee attendee;
  // This selects bitrate/resolution
  uint32_t stream_id = 0;
  uint32_t group_id = 0;
  uint32_t max_bitrate_kbps = 0;
};

}  //  namespace chime
#endif  // CHIME_SIGNALING_INTERNAL_LOCAL_STREAM_CONFIGURATION_H_
