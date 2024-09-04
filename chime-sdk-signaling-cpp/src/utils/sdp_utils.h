//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
/* @unstable */

#ifndef CHIME_SIGNALING_STRING_UTILS_H_
#define CHIME_SIGNALING_STRING_UTILS_H_

#include "audio_video/media_section.h"

#include <string>
#include <regex>

namespace chime {

const std::string rec_only = "a=recvonly";
const std::string send_only = "a=sendonly";
const std::string inactive = "a=inactive";
const std::string sendrecv = "a=sendrecv";

class SDPUtils {
 public:
  static std::string RemoveFirstOccurrence(std::string& str, const std::string& word) {
    auto pos = str.find(word);
    if (pos == std::string::npos) return str;

    str.erase(pos, word.length());
    return str;
  }

  static std::string RemoveAllSinceWordOccurrence(std::string& str, const std::string& word) {
    auto pos = str.find(word);
    if (pos == std::string::npos) return str;
    str.erase(str.begin() + static_cast<int>(pos), str.end());
    return str;
  }

  static std::vector<std::string> Split(const std::string& str, const std::string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
      token = str.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + delim_len;
      res.push_back(token);
    }

    res.push_back(str.substr(pos_start));
    return res;
  }

  // TODO: might be worth if we don't do parsing by ourselves.
  static std::vector<MediaSection> ParseSDP(const std::string& sdp) {
    std::vector<std::string> sdp_vec = Split(sdp, "\r\n");
    std::vector<MediaSection> media_sections;
    const std::string mid_str = "a=mid:";
    bool is_audio_section = false;
    std::string mid;
    for (const auto& line : sdp_vec) {
      if (line.find("m=audio", 0) == 0) {
        is_audio_section = true;
      } else if (line.find("m=video", 0) == 0) {
        is_audio_section = false;
      } else if (line.find(mid_str, 0) == 0) {
        mid = line.substr(mid_str.length(), std::string::npos);
      } else if (line.find(rec_only, 0) == 0 || line.find(send_only, 0) == 0 || line.find(inactive, 0) == 0 ||
                 line.find(sendrecv, 0) == 0) {
        MediaSection media_section{is_audio_section ? MediaType::kAudio : MediaType::kVideo, mid, GetDirection(line)};

        media_sections.emplace_back(media_section);
      }
    }

    return media_sections;
  }

 private:
  static MediaDirection GetDirection(const std::string& str) {
    if (str == rec_only) return MediaDirection::kRecvOnly;
    if (str == inactive) return MediaDirection::kInactive;
    if (str == send_only) return MediaDirection::kSendOnly;

    return MediaDirection::kSendRecv;
  }
};

}  // namespace chime
#endif  // CHIME_SIGNALING_STRING_UTILS_H_
