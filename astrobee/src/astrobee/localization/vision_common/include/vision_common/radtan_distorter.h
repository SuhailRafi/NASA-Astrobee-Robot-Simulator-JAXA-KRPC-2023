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
#ifndef VISION_COMMON_RADTAN_DISTORTER_H_
#define VISION_COMMON_RADTAN_DISTORTER_H_

#include <vision_common/distorter.h>
#include <vision_common/utilities.h>

#include <Eigen/Core>

#include <opencv2/core/eigen.hpp>

#include <ceres/ceres.h>

namespace vision_common {
class RadTanDistorter : public Distorter<4, RadTanDistorter> {
 public:
  using Distorter<4, RadTanDistorter>::Distort;
  using Distorter<4, RadTanDistorter>::Undistort;

  template <typename T>
  Eigen::Matrix<T, 2, 1> Distort(const T* distortion, const Eigen::Matrix<T, 3, 3>& intrinsics,
                                 const Eigen::Matrix<T, 2, 1>& undistorted_point) const {
    const T& k1 = distortion[0];
    const T& k2 = distortion[1];
    const T& p1 = distortion[2];
    const T& p2 = distortion[3];
    // TODO(rsoussan): Support 5 distortion params
    const T k3(0.0);

    const Eigen::Matrix<T, 2, 1> relative_coordinates = RelativeCoordinates(undistorted_point, intrinsics);
    const T& relative_x = relative_coordinates[0];
    const T& relative_y = relative_coordinates[1];
    // Squared norm
    const T r2 = relative_x * relative_x + relative_y * relative_y;

    // Apply radial distortion
    const T radial_distortion_coeff = 1.0 + k1 * r2 + k2 * r2 * r2 + k3 * r2 * r2 * r2;
    T distorted_relative_x = relative_x * radial_distortion_coeff;
    T distorted_relative_y = relative_y * radial_distortion_coeff;

    // Apply tangential distortion
    distorted_relative_x =
      distorted_relative_x + (2.0 * p1 * relative_x * relative_y + p2 * (r2 + 2.0 * relative_x * relative_x));
    distorted_relative_y =
      distorted_relative_y + (p1 * (r2 + 2.0 * relative_y * relative_y) + 2.0 * p2 * relative_x * relative_y);

    return AbsoluteCoordinates(Eigen::Matrix<T, 2, 1>(distorted_relative_x, distorted_relative_y), intrinsics);
  }

  cv::Mat Undistort(const cv::Mat& distorted_image, const Eigen::Matrix3d& intrinsics,
                    const Eigen::VectorXd& distortion) const final {
    cv::Mat undistorted_image;
    cv::Mat intrinsics_mat;
    cv::eigen2cv(intrinsics, intrinsics_mat);
    cv::Mat distortion_vector;
    cv::eigen2cv(distortion, distortion_vector);
    cv::undistort(distorted_image, undistorted_image, intrinsics_mat, distortion_vector);
    return undistorted_image;
  }

  Eigen::Vector2d Undistort(const Eigen::Vector2d& distorted_point, const Eigen::Matrix3d& intrinsics,
                            const Eigen::VectorXd& distortion) const final {
    cv::Mat intrinsics_mat;
    cv::eigen2cv(intrinsics, intrinsics_mat);
    cv::Mat distortion_vector;
    cv::eigen2cv(distortion, distortion_vector);
    cv::Mat distorted_point_vector;
    cv::eigen2cv(distorted_point, distorted_point_vector);
    cv::Mat undistorted_point_vector;
    cv::undistort(distorted_point_vector, undistorted_point_vector, intrinsics_mat, distortion_vector);
    Eigen::Vector2d undistorted_point;
    cv::cv2eigen(undistorted_point_vector, undistorted_point);
    return undistorted_point;
  }
};
}  // namespace vision_common

#endif  // VISION_COMMON_RADTAN_DISTORTER_H_
