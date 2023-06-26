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
#ifndef VISION_COMMON_FEATURE_MATCH_H_
#define VISION_COMMON_FEATURE_MATCH_H_

#include <Eigen/Core>

#include <vector>

namespace vision_common {
struct FeatureMatch {
  FeatureMatch(const Eigen::Vector2d& source_point, const Eigen::Vector2d& target_point, const double distance)
      : source_point(source_point), target_point(target_point), distance(distance) {}
  Eigen::Vector2d source_point;
  Eigen::Vector2d target_point;
  double distance;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
using FeatureMatches = std::vector<FeatureMatch>;
}  // namespace vision_common

#endif  // VISION_COMMON_FEATURE_MATCH_H_
