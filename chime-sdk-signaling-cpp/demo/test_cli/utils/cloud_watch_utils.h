//  Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.

#ifndef CLOUD_WATCH_UTILS_H_
#define CLOUD_WATCH_UTILS_H_

#include <unordered_map>
#include <string>
#include <aws/monitoring/model/PutMetricDataRequest.h>

class CloudWatchUtils {
 public:
  static bool SendMetricsSuccessFail(const std::string& space, const std::string& name, bool is_success);

 private:
  static bool SendMetrics(const std::string& space, const Aws::CloudWatch::Model::MetricDatum& datum);
};

#endif  // CLOUD_WATCH_UTILS_H_