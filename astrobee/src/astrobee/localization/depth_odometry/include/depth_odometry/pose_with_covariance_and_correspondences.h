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
#ifndef DEPTH_ODOMETRY_POSE_WITH_COVARIANCE_AND_CORRESPONDENCES_H_
#define DEPTH_ODOMETRY_POSE_WITH_COVARIANCE_AND_CORRESPONDENCES_H_

#include <point_cloud_common/icp_correspondences.h>
#include <localization_common/pose_with_covariance.h>
#include <localization_common/time.h>
#include <localization_measurements/depth_correspondences.h>

namespace depth_odometry {
struct PoseWithCovarianceAndCorrespondences {
  PoseWithCovarianceAndCorrespondences(const localization_common::PoseWithCovariance& pose_with_covariance,
                                       const point_cloud_common::ICPCorrespondences& correspondences,
                                       const localization_common::Time source_time,
                                       const localization_common::Time target_time)
      : pose_with_covariance(pose_with_covariance),
        depth_correspondences(correspondences.source_points, correspondences.target_points),
        source_time(source_time),
        target_time(target_time) {}

  PoseWithCovarianceAndCorrespondences(const localization_common::PoseWithCovariance& pose_with_covariance,
                                       const localization_measurements::DepthCorrespondences& correspondences,
                                       const localization_common::Time source_time,
                                       const localization_common::Time target_time)
      : pose_with_covariance(pose_with_covariance),
        depth_correspondences(correspondences),
        source_time(source_time),
        target_time(target_time) {}

  localization_common::PoseWithCovariance pose_with_covariance;
  localization_measurements::DepthCorrespondences depth_correspondences;
  localization_common::Time source_time;
  localization_common::Time target_time;
};
}  // namespace depth_odometry

#endif  // DEPTH_ODOMETRY_POSE_WITH_COVARIANCE_AND_CORRESPONDENCES_H_
