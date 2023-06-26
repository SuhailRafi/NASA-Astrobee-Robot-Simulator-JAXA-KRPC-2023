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

#include <localization_common/utilities.h>
#include <vision_common/pose_estimation.h>

#include <opencv2/core/eigen.hpp>

#include <iostream>
#include <random>
#include <unordered_map>

namespace vision_common {
namespace lc = localization_common;

void UndistortedPnP(const std::vector<cv::Point2d>& undistorted_image_points, const std::vector<cv::Point3d>& points_3d,
                    const cv::Mat& intrinsics, const int pnp_method, cv::Mat& rotation, cv::Mat& translation) {
  cv::Mat zero_distortion(4, 1, cv::DataType<double>::type, cv::Scalar(0));
  cv::solvePnP(points_3d, undistorted_image_points, intrinsics, zero_distortion, rotation, translation, false,
               pnp_method);
}

std::vector<int> RandomNIndices(const int num_possible_indices, const int num_sampled_indices) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0, num_possible_indices - 1);
  std::unordered_set<int> sampled_indices_set;
  std::vector<int> sampled_indices;
  while (static_cast<int>(sampled_indices.size()) < num_sampled_indices) {
    const int random_index = distribution(gen);
    if (sampled_indices_set.count(random_index) > 0) continue;
    sampled_indices_set.emplace(random_index);
    sampled_indices.emplace_back(random_index);
  }
  return sampled_indices;
}
}  // namespace vision_common
