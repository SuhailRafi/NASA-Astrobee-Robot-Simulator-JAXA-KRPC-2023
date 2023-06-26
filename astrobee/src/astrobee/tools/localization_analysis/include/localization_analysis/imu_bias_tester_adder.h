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

#ifndef LOCALIZATION_ANALYSIS_IMU_BIAS_TESTER_ADDER_H_
#define LOCALIZATION_ANALYSIS_IMU_BIAS_TESTER_ADDER_H_

#include <imu_bias_tester/imu_bias_tester_wrapper.h>

#include <rosbag/bag.h>
#include <rosbag/view.h>

#include <string>

namespace localization_analysis {
class ImuBiasTesterAdder {
 public:
  ImuBiasTesterAdder(const std::string& input_bag_name, const std::string& output_bag_name);
  void AddPredictions();

 private:
  imu_bias_tester::ImuBiasTesterWrapper imu_bias_tester_wrapper_;
  rosbag::Bag input_bag_;
  rosbag::Bag output_bag_;
};
}  // end namespace localization_analysis

#endif  // LOCALIZATION_ANALYSIS_IMU_BIAS_TESTER_ADDER_H_
