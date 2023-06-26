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

#ifndef GRAPH_LOCALIZER_SMART_PROJECTION_FACTOR_ADDER_PARAMS_H_
#define GRAPH_LOCALIZER_SMART_PROJECTION_FACTOR_ADDER_PARAMS_H_

#include <graph_optimizer/factor_adder_params.h>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/linear/NoiseModel.h>

#include <string>

namespace graph_localizer {
struct SmartProjectionFactorAdderParams : public graph_optimizer::FactorAdderParams {
  double min_avg_distance_from_mean;
  bool enable_EPI;
  double landmark_distance_threshold;
  double dynamic_outlier_rejection_threshold;
  double retriangulation_threshold;
  bool verbose_cheirality;
  bool robust;
  int max_num_factors;
  int min_num_points;
  int max_num_points_per_factor;
  int measurement_spacing;
  double feature_track_min_separation;
  bool rotation_only_fallback;
  bool splitting;
  bool scale_noise_with_num_points;
  double noise_scale;
  bool use_allowed_timestamps;
  gtsam::Pose3 body_T_cam;
  boost::shared_ptr<gtsam::Cal3_S2> cam_intrinsics;
  gtsam::SharedIsotropic cam_noise;
};
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_SMART_PROJECTION_FACTOR_ADDER_PARAMS_H_
