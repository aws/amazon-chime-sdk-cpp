//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
#include "utils/test_marker.h"

void TestMarker::MarkSuccessful(TestCase test_case) { Mark(test_case, true); }

void TestMarker::MarkFailed(TestCase test_case) { Mark(test_case, false); }

void TestMarker::Mark(TestCase test_case, bool successful) { marker_[test_case] = successful; }

bool TestMarker::IsSuccess(TestCase test_case) {
  if (!IsProcessed(test_case)) return false;
  return marker_[test_case];
}

bool TestMarker::IsProcessed(TestCase test_case) { return marker_.find(test_case) != marker_.end(); }