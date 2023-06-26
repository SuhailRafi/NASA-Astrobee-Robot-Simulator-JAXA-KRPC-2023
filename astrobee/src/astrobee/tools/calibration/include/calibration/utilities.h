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
#ifndef CALIBRATION_UTILITIES_H_
#define CALIBRATION_UTILITIES_H_

#include <calibration/match_set.h>
#include <ff_common/eigen_vectors.h>
#include <localization_common/image_correspondences.h>
#include <optimization_common/utilities.h>
#include <vision_common/pose_estimation.h>

#include <Eigen/Geometry>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace calibration {
void PrintCameraTTargetsStats(const std::vector<MatchSet>& match_sets,
                              const std::vector<Eigen::Isometry3d>& optimized_camera_T_targets);

template <typename DISTORTER>
void SaveReprojectionImage(const std::vector<Eigen::Vector2d>& image_points,
                           const std::vector<Eigen::Vector3d>& points_3d, const std::vector<int>& indices,
                           const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
                           const Eigen::Isometry3d& pose, const double max_visualization_error_norm,
                           const std::string& name);

template <typename DISTORTER>
void SaveReprojectionFromAllTargetsImage(const std::vector<Eigen::Isometry3d>& camera_T_targets,
                                         const std::vector<localization_common::ImageCorrespondences>& valid_match_sets,
                                         const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
                                         const Eigen::Vector2i& image_size,
                                         const double max_visualization_error_norm = 100);

int ErrorColor(const double error, const double max_error, const double max_color_value);

cv::Mat MapImageColors(const cv::Mat& gray_image);

template <typename DISTORTER>
void SaveReprojectionImage(const std::vector<Eigen::Vector2d>& image_points,
                           const std::vector<Eigen::Vector3d>& points_3d, const std::vector<int>& indices,
                           const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
                           const Eigen::Isometry3d& pose, const double max_visualization_error_norm,
                           const std::string& name) {
  cv::Mat reprojection_image_grayscale(960, 1280, CV_8UC1, cv::Scalar(0));
  std::unordered_set<int> inlier_indices(indices.begin(), indices.end());
  for (int i = 0; i < static_cast<int>(image_points.size()); ++i) {
    const Eigen::Vector2d& image_point = image_points[i];
    const Eigen::Vector3d& point_3d = points_3d[i];
    const Eigen::Vector3d camera_t_target_point = pose * point_3d;
    const Eigen::Vector2d projected_image_point =
      vision_common::ProjectWithDistortion<DISTORTER>(camera_t_target_point, intrinsics, distortion);
    const Eigen::Vector2d error = (image_point - projected_image_point);
    const double error_norm = error.norm();
    const cv::Point2i rounded_image_point(std::round(image_point.x()), std::round(image_point.y()));
    // Add 1 to each value so background pixels stay white and we can map these back to white
    // after applying colormap.
    // Only map up to 235 since darker reds that occur from 235-255 are hard to differentiate from
    // darker blues from 0 to 20 or so.
    constexpr double max_color_value = 235.0;
    const int error_color = ErrorColor(error_norm, max_visualization_error_norm, max_color_value) + 1;
    if (inlier_indices.count(i) > 0) {
      cv::circle(reprojection_image_grayscale, rounded_image_point, 4, cv::Scalar(235), -1);
      cv::circle(reprojection_image_grayscale,
                 cv::Point2i(std::round(projected_image_point.x()), std::round(projected_image_point.y())), 4,
                 cv::Scalar(error_color), -1);
    } else {  // Draw outlier with a triangle
      cv::drawMarker(reprojection_image_grayscale, rounded_image_point, cv::Scalar(235), cv::MARKER_TRIANGLE_DOWN, 6,
                     2);
      cv::drawMarker(reprojection_image_grayscale,
                     cv::Point2i(std::round(projected_image_point.x()), std::round(projected_image_point.y())),
                     cv::Scalar(error_color), cv::MARKER_TRIANGLE_DOWN, 6, 2);
    }
  }

  const cv::Mat reprojection_image_color = MapImageColors(reprojection_image_grayscale);
  cv::imwrite(name, reprojection_image_color);
}

template <typename DISTORTER>
void SaveReprojectionFromAllTargetsImage(const std::vector<Eigen::Isometry3d>& camera_T_targets,
                                         const std::vector<localization_common::ImageCorrespondences>& valid_match_sets,
                                         const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
                                         const Eigen::Vector2i& image_size, const double max_visualization_error_norm) {
  double max_error_norm = 0;
  // Absolute Image
  cv::Mat absolute_reprojection_image_grayscale(image_size.y(), image_size.x(), CV_8UC1, cv::Scalar(0));
  std::ofstream errors_file;
  errors_file.open("errors_file.txt");
  for (int i = 0; i < static_cast<int>(valid_match_sets.size()); ++i) {
    const auto& match_set = valid_match_sets[i];
    const Eigen::Isometry3d camera_T_target = camera_T_targets[i];
    for (int j = 0; j < static_cast<int>(match_set.size()); ++j) {
      const auto& image_point = match_set.image_points[j];
      const auto& target_point = match_set.points_3d[j];
      const Eigen::Vector3d camera_t_target_point = camera_T_target * target_point;
      const Eigen::Vector2d projected_image_point =
        vision_common::ProjectWithDistortion<DISTORTER>(camera_t_target_point, intrinsics, distortion);
      const Eigen::Vector2d error = (image_point - projected_image_point);
      const double error_norm = error.norm();
      if (error_norm > max_error_norm) max_error_norm = error_norm;
      errors_file << error.x() << " " << error.y() << std::endl;
      const cv::Point2i rounded_image_point(std::round(image_point.x()), std::round(image_point.y()));
      // Add 1 to each value so background pixels stay white and we can map these back to white
      // after applying colormap.
      // Only map up to 235 since darker reds that occur from 235-255 are hard to differentiate from
      // darker blues from 0 to 20 or so.
      constexpr double max_color_value = 235.0;
      const int absolute_error_color = ErrorColor(error_norm, max_visualization_error_norm, max_color_value) + 1;
      cv::circle(absolute_reprojection_image_grayscale, rounded_image_point, 4, cv::Scalar(absolute_error_color), -1);
    }
  }
  const cv::Mat absolute_reprojection_image_color = MapImageColors(absolute_reprojection_image_grayscale);
  cv::imwrite("calibrated_reprojection_from_all_targets_absolute_image.png", absolute_reprojection_image_color);
  errors_file.close();

  // Relative Image
  cv::Mat relative_reprojection_image_grayscale(image_size.y(), image_size.x(), CV_8UC1, cv::Scalar(0));
  for (int i = 0; i < static_cast<int>(valid_match_sets.size()); ++i) {
    const auto& match_set = valid_match_sets[i];
    const Eigen::Isometry3d camera_T_target = camera_T_targets[i];
    for (int j = 0; j < static_cast<int>(match_set.size()); ++j) {
      const auto& image_point = match_set.image_points[j];
      const auto& target_point = match_set.points_3d[j];
      const Eigen::Vector3d camera_t_target_point = camera_T_target * target_point;
      const Eigen::Vector2d projected_image_point =
        vision_common::ProjectWithDistortion<DISTORTER>(camera_t_target_point, intrinsics, distortion);
      const Eigen::Vector2d error = (image_point - projected_image_point);
      const double error_norm = error.norm();
      const cv::Point2i rounded_image_point(std::round(image_point.x()), std::round(image_point.y()));
      // Add 1 to each value so background pixels stay white and we can map these back to white
      // after applying colormap.
      // Only map up to 235 since darker reds that occur from 235-255 are hard to differentiate from
      // darker blues from 0 to 20 or so.
      constexpr double max_color_value = 235.0;
      const int relative_error_color = ErrorColor(error_norm, max_error_norm, max_color_value) + 1;
      cv::circle(relative_reprojection_image_grayscale, rounded_image_point, 4, cv::Scalar(relative_error_color), -1);
    }
  }
  const cv::Mat relative_reprojection_image_color = MapImageColors(relative_reprojection_image_grayscale);
  cv::imwrite("calibrated_reprojection_from_all_targets_relative_image.png", relative_reprojection_image_color);
}
}  // namespace calibration

#endif  // CALIBRATION_UTILITIES_H_
