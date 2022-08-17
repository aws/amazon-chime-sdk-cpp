
//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#include "utils/cloud_watch_utils.h"
#include <aws/monitoring/CloudWatchClient.h>

// Try to get OS name based on macro, so it will get in the compile time
std::string OSName() {
#ifdef _WIN32
  return "Windows 32-bit";
#elif _WIN64
  return "Windows 64-bit";
#elif __APPLE__ || __MACH__
  return "Mac OSX";
#elif __linux__
  return "Linux";
#elif __FreeBSD__
  return "FreeBSD";
#elif __unix || __unix__
  return "Unix";
#else
  return "Other";
#endif
}

// static
bool CloudWatchUtils::SendMetricsSuccessFail(const std::string& space, const std::string& name, bool is_success) {
  // Add data to be displayed on the cloudwatch
  Aws::CloudWatch::Model::MetricDatum datum;
  datum.SetMetricName(name);
  datum.SetUnit(Aws::CloudWatch::Model::StandardUnit::None);
  datum.SetValue(is_success ? 1.0 : 0.0);

  // Send the metrics as aggregated so that we can put alarm on all data
  SendMetrics(space, datum);

  // These properties will appear in Cloudwatch on the point
  // Some point that is worth is might be OS where it is coming from.
  std::unordered_map<std::string, std::string> property_map;
  property_map["OS"] = OSName();
  property_map["From"] = "Canary";
  for (const auto& property : property_map) {
    Aws::CloudWatch::Model::Dimension dimension;
    dimension.SetName(property.first);
    dimension.SetValue(property.second);
    datum.AddDimensions(dimension);
  }

  return SendMetrics(space, datum);
}

// static
bool CloudWatchUtils::SendMetrics(const std::string& space, const Aws::CloudWatch::Model::MetricDatum& datum) {
  Aws::CloudWatch::CloudWatchClient cw;
  // Create request and send data
  Aws::CloudWatch::Model::PutMetricDataRequest request;
  request.SetNamespace(space);
  request.AddMetricData(datum);

  auto outcome = cw.PutMetricData(request);
  if (!outcome.IsSuccess()) {
    std::cout << "Failed to put sample metric data:" << outcome.GetError().GetMessage() << std::endl;
    return false;
  } else {
    std::cout << "Successfully put sample metric data" << std::endl;
    return true;
  }
}
