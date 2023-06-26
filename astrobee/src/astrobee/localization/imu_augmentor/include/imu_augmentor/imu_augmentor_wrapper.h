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
#ifndef IMU_AUGMENTOR_IMU_AUGMENTOR_WRAPPER_H_
#define IMU_AUGMENTOR_IMU_AUGMENTOR_WRAPPER_H_

#include <ff_msgs/EkfState.h>
#include <ff_msgs/FlightMode.h>
#include <ff_msgs/GraphState.h>
#include <imu_augmentor/imu_augmentor.h>
#include <localization_common/combined_nav_state.h>
#include <localization_common/combined_nav_state_covariances.h>
#include <localization_common/rate_timer.h>
#include <localization_measurements/imu_measurement.h>

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <sensor_msgs/Imu.h>

#include <memory>
#include <string>
#include <utility>

namespace imu_augmentor {
class ImuAugmentorWrapper {
 public:
  explicit ImuAugmentorWrapper(const std::string& graph_config_path_prefix = "");

  explicit ImuAugmentorWrapper(const ImuAugmentorParams& params);

  void LocalizationStateCallback(const ff_msgs::GraphState& loc_msg);

  void ImuCallback(const sensor_msgs::Imu& imu_msg);

  void FlightModeCallback(const ff_msgs::FlightMode& flight_mode);

  boost::optional<std::pair<localization_common::CombinedNavState, localization_common::CombinedNavStateCovariances>>
  LatestImuAugmentedCombinedNavStateAndCovariances();

  boost::optional<ff_msgs::EkfState> LatestImuAugmentedLocalizationMsg();

 private:
  void Initialize(const ImuAugmentorParams& params);

  bool LatestImuAugmentedCombinedNavStateAndCovariances(
    localization_common::CombinedNavState& latest_imu_augmented_combined_nav_state,
    localization_common::CombinedNavStateCovariances& latest_imu_augmented_covariances);

  bool LatestImuMeasurement(localization_measurements::ImuMeasurement& latest_imu_measurement);

  bool standstill() const;

  std::unique_ptr<ImuAugmentor> imu_augmentor_;
  boost::optional<localization_common::CombinedNavState> latest_combined_nav_state_;
  boost::optional<localization_common::CombinedNavState> latest_imu_augmented_combined_nav_state_;
  boost::optional<localization_common::CombinedNavStateCovariances> latest_covariances_;
  boost::optional<ff_msgs::GraphState> latest_loc_msg_;
  std::unique_ptr<gtsam::TangentPreintegration> preintegration_helper_;
  ImuAugmentorParams params_;
  boost::optional<bool> standstill_;
  localization_common::RateTimer loc_state_timer_ = localization_common::RateTimer("Loc State Msg");
};
}  // namespace imu_augmentor
#endif  // IMU_AUGMENTOR_IMU_AUGMENTOR_WRAPPER_H_
