/* Copyright (c) 2017, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 *
 * All rights reserved.
 *
 * The Astrobee platform is licensed under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with the
 * License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef LOCALIZATION_COMMON_RATE_TIMER_H_
#define LOCALIZATION_COMMON_RATE_TIMER_H_

#include <localization_common/averager.h>
#include <localization_common/time.h>

#include <boost/optional.hpp>

#include <chrono>
#include <string>

namespace localization_common {
class RateTimer {
 public:
  explicit RateTimer(const std::string& timer_name = "");
  boost::optional<double> LastValue();
  void Record();
  void Log() const;
  void LogEveryN(const int num_events_per_log) const;
  void VlogEveryN(const int num_events_per_log, const int level) const;
  void RecordAndLog();
  void RecordAndLogEveryN(const int num_events_per_log);
  void RecordAndVlogEveryN(const int num_events_per_log, const int level);
  void Vlog(const int level = 2) const;

 private:
  std::chrono::time_point<std::chrono::steady_clock> start_time_;
  Averager averager_;
  int num_events_;
};
}  // namespace localization_common

#endif  // LOCALIZATION_COMMON_RATE_TIMER_H_
