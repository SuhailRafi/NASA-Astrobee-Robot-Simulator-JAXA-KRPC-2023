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

#ifndef IMU_AUGMENTOR_IMU_AUGMENTOR_H_
#define IMU_AUGMENTOR_IMU_AUGMENTOR_H_

#include <imu_augmentor/imu_augmentor_params.h>
#include <imu_integration/imu_integrator.h>
#include <localization_common/combined_nav_state.h>

#include <string>

namespace imu_augmentor {
class ImuAugmentor : public imu_integration::ImuIntegrator {
 public:
  explicit ImuAugmentor(const ImuAugmentorParams& params);

  void PimPredict(const localization_common::CombinedNavState& latest_combined_nav_state,
                  localization_common::CombinedNavState& latest_imu_augmented_combined_nav_state);
};
}  // namespace imu_augmentor

#endif  // IMU_AUGMENTOR_IMU_AUGMENTOR_H_
