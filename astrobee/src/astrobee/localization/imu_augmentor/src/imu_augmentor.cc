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

#include <imu_augmentor/imu_augmentor.h>
#include <imu_integration/utilities.h>
#include <localization_common/logger.h>

namespace imu_augmentor {
namespace ii = imu_integration;
namespace lc = localization_common;
namespace lm = localization_measurements;
ImuAugmentor::ImuAugmentor(const ImuAugmentorParams& params) : ii::ImuIntegrator(params) {}

void ImuAugmentor::PimPredict(const lc::CombinedNavState& latest_combined_nav_state,
                              lc::CombinedNavState& latest_imu_augmented_combined_nav_state) {
  if (Empty()) return;
  // Start with least upper bound measurement
  // Don't add measurements with same timestamp as start_time
  // since these would have a dt of 0 (wrt the pim start time) and cause errors for the pim
  auto measurement_it = measurements().upper_bound(latest_imu_augmented_combined_nav_state.timestamp());
  if (measurement_it == measurements().cend()) return;
  // Always use the biases from the lastest combined nav state
  auto pim = ii::Pim(latest_combined_nav_state.bias(), pim_params());
  int num_measurements_added = 0;
  // Create new pim each time since pim uses beginning orientation and velocity for
  // gravity integration and initial velocity integration.
  for (; measurement_it != measurements().cend(); ++measurement_it) {
    pim.resetIntegrationAndSetBias(latest_combined_nav_state.bias());
    auto time = latest_imu_augmented_combined_nav_state.timestamp();
    ii::AddMeasurement(measurement_it->second, time, pim);
    latest_imu_augmented_combined_nav_state = ii::PimPredict(latest_imu_augmented_combined_nav_state, pim);
    ++num_measurements_added;
  }

  // Only remove measurements up to latest combined nav state so that when a new nav state is received IMU data is still
  // available for extrapolation
  RemoveOldMeasurements(latest_combined_nav_state.timestamp());
  LogDebug("PimPredict: Added " << num_measurements_added << " measurements.");
}
}  // namespace imu_augmentor
