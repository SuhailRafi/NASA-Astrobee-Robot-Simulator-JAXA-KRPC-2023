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

#include <calibration/utilities.h>
#include <ff_common/eigen_vectors.h>
#include <localization_common/averager.h>
#include <optimization_common/utilities.h>

#include <Eigen/Geometry>

namespace calibration {
namespace lc = localization_common;
namespace oc = optimization_common;

void PrintCameraTTargetsStats(const std::vector<MatchSet>& match_sets,
                              const std::vector<Eigen::Isometry3d>& optimized_camera_T_targets) {
  lc::Averager position_diff_norm_averager("Initial vs. optimized camera_T_target position diff norm");
  lc::Averager rotation_diff_averager("Initial vs. optimized camera_T_target rotation diff");
  for (int i = 0; i < static_cast<int>(match_sets.size()); ++i) {
    const Eigen::Isometry3d& initial_camera_T_target = match_sets[i].pose_estimate;
    const Eigen::Isometry3d& optimized_camera_T_target = optimized_camera_T_targets[i];
    const double position_diff_norm =
      (initial_camera_T_target.translation() - optimized_camera_T_target.translation()).norm();
    position_diff_norm_averager.Update(position_diff_norm);
    const Eigen::Matrix3d optimized_camera_R_initial_camera =
      (optimized_camera_T_target * initial_camera_T_target.inverse()).linear();
    const double rotation_diff = std::abs(Eigen::AngleAxisd(optimized_camera_R_initial_camera).angle());
    rotation_diff_averager.Update(rotation_diff);
  }
  position_diff_norm_averager.Log();
  rotation_diff_averager.Log();
}

int ErrorColor(const double error, const double max_error, const double max_color_value) {
  return std::round(std::min(error, max_error) / max_error * max_color_value);
}

cv::Mat MapImageColors(const cv::Mat& gray_image) {
  cv::Mat color_image;
  cv::applyColorMap(gray_image, color_image, cv::COLORMAP_JET);
  // Map white pixels back from lowest JET value (128, 0, 0) to white
  cv::Mat base_mask;
  cv::inRange(color_image, cv::Scalar(128, 0, 0), cv::Scalar(128, 0, 0), base_mask);
  color_image.setTo(cv::Scalar(255, 255, 255), base_mask);
  return color_image;
}
}  // namespace calibration
