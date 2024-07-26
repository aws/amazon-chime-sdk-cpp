// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CHIME_SIGNAL_FRAME_DEBUG_UTILS_H_
#define CHIME_SIGNAL_FRAME_DEBUG_UTILS_H_

#include "video_control_sdk.pb.h"


#include <string>

namespace chime {

class SignalFrameDebugUtils {
 public:
  static std::string DuplexToString(signal_sdk::SdkStreamServiceType duplex) {
    std::map<signal_sdk::SdkStreamServiceType, std::string> duplex_strings = {
        {signal_sdk::SdkStreamServiceType::RX, "RX"},
        {signal_sdk::SdkStreamServiceType::TX, "TX"},
        {signal_sdk::SdkStreamServiceType::DUPLEX, "DUPLEX"}};
    return duplex_strings[duplex];
  }

  static std::string SubscribeFrameDebugString(const signal_sdk::SdkSubscribeFrame& subscribe) {
    std::string str = "SignalFrame:\nSubscribe: {\nduplex: " + DuplexToString(subscribe.duplex()) +
                      "\naudio_host: " + subscribe.audio_host() +
                      "\naudio_checkin: " + std::to_string(subscribe.audio_checkin()) +
                      "\naudio_muted: " + std::to_string(subscribe.audio_muted()) + "\nallocations:{";

    for (const signal_sdk::SdkStreamDescriptor& stream : subscribe.send_streams()) {
      str += "\n\t";
      if (stream.has_group_id()) {
        str += "group_id:" + std::to_string(stream.group_id()) + " ";
      }
      if (stream.has_stream_id()) {
        str += "stream_id: " + std::to_string(stream.stream_id()) + " ";
      }
      if (stream.has_max_bitrate_kbps()) {
        str += "max_bitrate_kbps:" + std::to_string(stream.max_bitrate_kbps()) + " ";
      }
    }
    str += "\n}\n";

    str += "receive_streams:{";
    for (const uint32_t& stream_id : subscribe.receive_stream_ids()) {
      str += " " + std::to_string(stream_id);
    }
    str += " }\n";
    if (subscribe.has_sdp_offer()) {
      str += "SDP:\n" + subscribe.sdp_offer();
    }
    return str;
  }

  static std::string IndexFrameDebugString(const signal_sdk::SdkIndexFrame& index) {
    std::string str = "SignalFrame:\nIndex {\n";
    if (index.has_at_capacity()) {
      str += "at_capacity: " + std::to_string(index.at_capacity()) + "\n";
    }
    if (index.has_num_participants()) {
      str += "num_participants: " + std::to_string(index.num_participants()) + "\n";
    }
    for (const signal_sdk::SdkStreamDescriptor& stream : index.sources()) {
      str += "{\n";
      if (stream.has_group_id()) {
        str += "\tgroup_id: " + std::to_string(stream.group_id()) + "\n";
      }
      if (stream.has_stream_id()) {
        str += "\tstream_id: " + std::to_string(stream.stream_id()) + "\n";
      }
      if (stream.has_max_bitrate_kbps()) {
        str += "\tmax_bitrate_kbps: " + std::to_string(stream.max_bitrate_kbps()) + "\n";
      }
      if (stream.has_avg_bitrate_bps()) {
        str += "\tavg_bitrate_bps: " + std::to_string(stream.avg_bitrate_bps()) + "\n";
      }
      if (stream.has_attendee_id()) {
        str += "\tattendee_id: " + stream.attendee_id() + "\n";
      }
      str += "}\n";
    }
    str += "}\n";
    return str;
  }
};
}  // namespace chime

#endif  // CHIME_SIGNAL_FRAME_DEBUG_UTILS_H_
