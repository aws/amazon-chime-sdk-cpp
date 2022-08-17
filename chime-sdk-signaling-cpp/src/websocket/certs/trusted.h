//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNALING_TRUSTED_H_
#define CHIME_SIGNALING_TRUSTED_H_

#include <cstdint>

namespace chime {

struct Certificate {
  unsigned char *cert = nullptr;
  uint32_t len = 0;
};

extern Certificate all_prod_certs[5];


} // namespace chime

#endif  // CHIME_SIGNALING_TRUSTED_H_
