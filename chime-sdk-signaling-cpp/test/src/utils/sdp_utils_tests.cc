//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "gtest/gtest.h"

#include "utils/sdp_utils.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace chime;

class StringUtilsTest : public testing::Test {
 public:
  StringUtilsTest() {
    std::string line;
    std::stringstream buffer;
    std::string res;
    std::ifstream file("example_sdp.txt");
    if (file.is_open()) {
      buffer << file.rdbuf();
    } else {
      std::cout << "Unable to open file" << std::endl;
    }
    std::cout << "res: " << buffer.str() << std::endl;
    sdp_ = buffer.str();
  }

  std::string sdp_;
};

TEST_F(StringUtilsTest, ShouldBeAbleToParseSDP) {
  std::vector<MediaSection> sections = SDPUtils::ParseSDP(sdp_);
  std::vector<MediaSection> expected;

  MediaSection section {
      MediaType::kAudio,
      "0",
      MediaDirection::kSendRecv
  };
  expected.emplace_back(section);
  MediaSection section2 {
      MediaType::kVideo,
      "1",
      MediaDirection::kInactive
  };
  expected.emplace_back(section2);
  MediaSection section3 {
      MediaType::kVideo,
      "2",
      MediaDirection::kRecvOnly
  };
  expected.emplace_back(section3);

  ASSERT_EQ(expected.size(), sections.size());
  
  for (int i = 0; i < expected.size(); ++i) {
    MediaSection expected_section = expected[i];
    MediaSection result_section = sections[i];

    EXPECT_EQ(expected_section.direction, result_section.direction);
    EXPECT_EQ(expected_section.mid, result_section.mid);
    EXPECT_EQ(expected_section.type, result_section.type);
  }
}