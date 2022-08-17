//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef TEST_MARKER_H_
#define TEST_MARKER_H_

#include <unordered_map>

enum TestCase { kConnection, kSubscription, kVideo, kAudio };

class TestMarker {
 public:
  void MarkSuccessful(TestCase test_case);
  void MarkFailed(TestCase test_case);
  bool IsSuccess(TestCase test_case);
  bool IsProcessed(TestCase test_case);

 private:
  void Mark(TestCase test_case, bool isSuccess);
  std::unordered_map<TestCase, bool> marker_;
};

#endif  // TEST_MARKER_H_