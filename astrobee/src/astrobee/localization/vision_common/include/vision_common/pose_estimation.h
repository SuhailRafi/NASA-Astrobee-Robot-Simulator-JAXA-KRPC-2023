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
#ifndef VISION_COMMON_POSE_ESTIMATION_H_
#define VISION_COMMON_POSE_ESTIMATION_H_

#include <ff_common/eigen_vectors.h>
#include <localization_common/logger.h>
#include <localization_common/pose_with_covariance.h>
#include <localization_common/utilities.h>
#include <optimization_common/residuals.h>
#include <optimization_common/utilities.h>
#include <vision_common/pose_with_covariance_and_inliers.h>
#include <vision_common/ransac_pnp_params.h>
#include <vision_common/reprojection_pose_estimate_params.h>
#include <vision_common/utilities.h>

#include <Eigen/Geometry>

#include <ceres/ceres.h>
#include <ceres/solver.h>

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core/eigen.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace vision_common {
void UndistortedPnP(const std::vector<cv::Point2d>& undistorted_image_points, const std::vector<cv::Point3d>& points_3d,
                    const cv::Mat& intrinsics, const int pnp_method, cv::Mat& rotation, cv::Mat& translation);

std::vector<int> RandomNIndices(const int num_possible_indices, const int num_sampled_indices);

template <typename DISTORTER>
int Inliers(const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
            const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
            const Eigen::Isometry3d& pose_estimate, const double max_inlier_threshold,
            boost::optional<std::vector<int>&> inliers = boost::none);

template <typename T>
std::vector<T> SampledValues(const std::vector<T>& values, const std::vector<int>& indices);

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> RansacPnP(const std::vector<Eigen::Vector2d>& image_points,
                                                        const std::vector<Eigen::Vector3d>& points_3d,
                                                        const Eigen::Matrix3d& intrinsics,
                                                        const Eigen::VectorXd& distortion,
                                                        const RansacPnPParams& params);

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimate(const std::vector<Eigen::Vector2d>& image_points,
                                                                       const std::vector<Eigen::Vector3d>& points_3d,
                                                                       const Eigen::Vector2d& focal_lengths,
                                                                       const Eigen::Vector2d& principal_points,
                                                                       const Eigen::VectorXd& distortion,
                                                                       const ReprojectionPoseEstimateParams& params);

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimate(const std::vector<Eigen::Vector2d>& image_points,
                                                                       const std::vector<Eigen::Vector3d>& points_3d,
                                                                       const Eigen::Matrix3d& intrinsics,
                                                                       const Eigen::VectorXd& distortion,
                                                                       const ReprojectionPoseEstimateParams& params);

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimateWithInitialEstimate(
  const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
  const Eigen::Vector2d& focal_lengths, const Eigen::Vector2d& principal_points, const Eigen::VectorXd& distortion,
  const ReprojectionPoseEstimateParams& params, const Eigen::Isometry3d& initial_estimate,
  const std::vector<int>& initial_inliers);

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimateWithInitialEstimate(
  const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
  const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion, const ReprojectionPoseEstimateParams& params,
  const Eigen::Isometry3d& initial_estimate, const std::vector<int>& initial_inliers);

template <typename DISTORTER>
int Inliers(const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
            const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion,
            const Eigen::Isometry3d& pose_estimate, const double max_inlier_threshold,
            boost::optional<std::vector<int>&> inliers) {
  int num_inliers = 0;
  for (int i = 0; i < static_cast<int>(image_points.size()); ++i) {
    const Eigen::Vector2d& image_point = image_points[i];
    const Eigen::Vector3d& point_3d = points_3d[i];
    const Eigen::Vector3d transformed_point_3d = pose_estimate * point_3d;
    const Eigen::Vector2d projected_image_point =
      ProjectWithDistortion<DISTORTER>(transformed_point_3d, intrinsics, distortion);
    const Eigen::Vector2d error = (image_point - projected_image_point);
    const double error_norm = error.norm();
    if (error_norm <= max_inlier_threshold) {
      ++num_inliers;
      if (inliers) inliers->emplace_back(i);
    }
  }
  return num_inliers;
}

template <typename T>
std::vector<T> SampledValues(const std::vector<T>& values, const std::vector<int>& indices) {
  std::vector<T> sampled_values;
  for (const int i : indices) {
    sampled_values.emplace_back(values[i]);
  }
  return sampled_values;
}

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> RansacPnP(const std::vector<Eigen::Vector2d>& image_points,
                                                        const std::vector<Eigen::Vector3d>& points_3d,
                                                        const Eigen::Matrix3d& intrinsics,
                                                        const Eigen::VectorXd& distortion,
                                                        const RansacPnPParams& params) {
  if (image_points.size() < 4) {
    LogError("RansacPnP: Too few matched points given.");
    return boost::none;
  }

  const DISTORTER distorter;
  const std::vector<Eigen::Vector2d> undistorted_image_points =
    distorter.Undistort(image_points, intrinsics, distortion);

  std::vector<cv::Point2d> undistorted_image_points_cv;
  for (const auto& undistorted_image_point : undistorted_image_points) {
    undistorted_image_points_cv.emplace_back(cv::Point2d(undistorted_image_point.x(), undistorted_image_point.y()));
  }
  std::vector<cv::Point3d> points_3d_cv;
  for (const auto& point_3d : points_3d) {
    points_3d_cv.emplace_back(point_3d.x(), point_3d.y(), point_3d.z());
  }

  cv::Mat intrinsics_cv;
  cv::eigen2cv(intrinsics, intrinsics_cv);
  cv::Mat rodrigues_rotation_cv(3, 1, cv::DataType<double>::type, cv::Scalar(0));
  cv::Mat translation_cv(3, 1, cv::DataType<double>::type, cv::Scalar(0));
  int max_num_inliers = 0;
  Eigen::Isometry3d best_pose_estimate;
  // Use 4 values for P3P
  // TODO(rsoussan): make this a param
  constexpr int kNumSampledValues = 4;
  for (int i = 0; i < params.num_iterations; ++i) {
    const auto sampled_indices = RandomNIndices(image_points.size(), kNumSampledValues);
    const auto sampled_undistorted_image_points = SampledValues(undistorted_image_points_cv, sampled_indices);
    const auto sampled_points_3d = SampledValues(points_3d_cv, sampled_indices);
    UndistortedPnP(sampled_undistorted_image_points, sampled_points_3d, intrinsics_cv, params.pnp_method,
                   rodrigues_rotation_cv, translation_cv);
    const Eigen::Isometry3d pose_estimate = Isometry3d(rodrigues_rotation_cv, translation_cv);
    const int num_inliers =
      Inliers<DISTORTER>(image_points, points_3d, intrinsics, distortion, pose_estimate, params.max_inlier_threshold);
    if (num_inliers > max_num_inliers) {
      best_pose_estimate = pose_estimate;
      max_num_inliers = num_inliers;
    }
  }

  if (max_num_inliers < params.min_num_inliers) {
    LogError("RansacPnP: Failed to find pose with enough inliers.");
    return boost::none;
  }

  std::vector<int> inliers;
  Inliers<DISTORTER>(image_points, points_3d, intrinsics, distortion, best_pose_estimate, params.max_inlier_threshold,
                     inliers);
  // TODO(rsoussan): Get covariance for ransac pnp
  return PoseWithCovarianceAndInliers(best_pose_estimate, Eigen::Matrix<double, 6, 6>::Identity(), inliers);
}

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimate(const std::vector<Eigen::Vector2d>& image_points,
                                                                       const std::vector<Eigen::Vector3d>& points_3d,
                                                                       const Eigen::Vector2d& focal_lengths,
                                                                       const Eigen::Vector2d& principal_points,
                                                                       const Eigen::VectorXd& distortion,
                                                                       const ReprojectionPoseEstimateParams& params) {
  if (image_points.size() < 4) {
    LogError("ReprojectionPoseEstimate: Too few matched points given.");
    return boost::none;
  }

  const Eigen::Matrix3d intrinsics = optimization_common::Intrinsics(focal_lengths, principal_points);
  // Use RansacPnP for initial estimate since using identity transform can lead to image projection issues
  // if any points_3d z values are 0.
  const auto ransac_pnp_estimate =
    RansacPnP<DISTORTER>(image_points, points_3d, intrinsics, distortion, params.ransac_pnp);
  if (!ransac_pnp_estimate) {
    LogError("ReprojectionPoseEstimate: Failed to get ransac pnp initial estimate.");
    return boost::none;
  }

  const int num_inliers = ransac_pnp_estimate->inliers.size();
  if (num_inliers < params.ransac_pnp.min_num_inliers) {
    LogError("ReprojectionPoseEstimate: Too few inliers found. Need " << params.ransac_pnp.min_num_inliers << ", got "
                                                                      << num_inliers << ".");
    return boost::none;
  }

  return ReprojectionPoseEstimateWithInitialEstimate<DISTORTER>(
    image_points, points_3d, focal_lengths, principal_points, distortion, params, ransac_pnp_estimate->pose,
    ransac_pnp_estimate->inliers);
}

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimateWithInitialEstimate(
  const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
  const Eigen::Vector2d& focal_lengths, const Eigen::Vector2d& principal_points, const Eigen::VectorXd& distortion,
  const ReprojectionPoseEstimateParams& params, const Eigen::Isometry3d& initial_estimate,
  const std::vector<int>& initial_inliers) {
  if (image_points.size() < 4) {
    LogError("ReprojectionPoseEstimateWithInitialEstimate: Too few matched points given.");
    return boost::none;
  }

  const int num_initial_inliers = initial_inliers.size();
  if (num_initial_inliers < params.ransac_pnp.min_num_inliers) {
    LogError("ReprojectionPoseEstimateWithInitialEstimate: Too few inliers provided. Need "
             << params.ransac_pnp.min_num_inliers << ", got " << num_initial_inliers << ".");
    return boost::none;
  }

  // TODO(rsoussan): Get covariance from ransac pnp
  if (!params.optimize_estimate)
    return PoseWithCovarianceAndInliers(initial_estimate, Eigen::Matrix<double, 6, 6>::Identity(), initial_inliers);

  ceres::Problem problem;
  optimization_common::AddConstantParameterBlock(2, focal_lengths.data(), problem);
  optimization_common::AddConstantParameterBlock(2, principal_points.data(), problem);
  optimization_common::AddConstantParameterBlock(DISTORTER::kNumParams, distortion.data(), problem);

  Eigen::Matrix<double, 6, 1> pose_estimate_vector = optimization_common::VectorFromIsometry3d(initial_estimate);
  problem.AddParameterBlock(pose_estimate_vector.data(), 6);

  for (int i = 0; i < num_initial_inliers; ++i) {
    const int inlier_index = initial_inliers[i];
    optimization_common::ReprojectionError<DISTORTER>::AddCostFunction(
      image_points[inlier_index], points_3d[inlier_index], pose_estimate_vector,
      const_cast<Eigen::Vector2d&>(focal_lengths), const_cast<Eigen::Vector2d&>(principal_points),
      const_cast<Eigen::VectorXd&>(distortion), problem, 1, params.optimization.huber_loss);
  }

  ceres::Solver::Summary summary;
  ceres::Solve(params.optimization.solver_options, &problem, &summary);
  const Eigen::Isometry3d optimized_estimate = optimization_common::Isometry3d(pose_estimate_vector);
  if (params.optimization.verbose) {
    LogError("ReprojectionPoseEstimateWithInitialEstimate: Initial estimate: " << std::endl
                                                                               << initial_estimate.matrix() << std::endl
                                                                               << "Optimized estimate: " << std::endl
                                                                               << optimized_estimate.matrix()
                                                                               << std::endl
                                                                               << "Summary: " << std::endl
                                                                               << summary.FullReport());
  }
  if (!summary.IsSolutionUsable()) {
    LogError("ReprojectionPoseEstimateWithInitialEstimate: Failed to find solution.");
    return boost::none;
  }

  ceres::Covariance::Options options;
  ceres::Covariance covariance(options);
  std::vector<std::pair<const double*, const double*>> covariance_blocks;
  covariance_blocks.push_back(std::make_pair(pose_estimate_vector.data(), pose_estimate_vector.data()));
  if (!covariance.Compute(covariance_blocks, &problem)) {
    LogError("ReprojectionPoseEstimateWithInitialEstimate: Failed to compute covariances.");
    return boost::none;
  }

  localization_common::PoseCovariance pose_covariance;
  covariance.GetCovarianceBlock(pose_estimate_vector.data(), pose_estimate_vector.data(), pose_covariance.data());

  std::vector<int> inliers;
  const Eigen::Matrix3d intrinsics = optimization_common::Intrinsics(focal_lengths, principal_points);
  Inliers<DISTORTER>(image_points, points_3d, intrinsics, distortion, optimized_estimate, params.max_inlier_threshold,
                     inliers);
  return PoseWithCovarianceAndInliers(optimized_estimate, pose_covariance, inliers);
}

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimate(const std::vector<Eigen::Vector2d>& image_points,
                                                                       const std::vector<Eigen::Vector3d>& points_3d,
                                                                       const Eigen::Matrix3d& intrinsics,
                                                                       const Eigen::VectorXd& distortion,
                                                                       const ReprojectionPoseEstimateParams& params) {
  const auto focal_lengths = FocalLengths(intrinsics);
  const auto principal_points = PrincipalPoints(intrinsics);
  return ReprojectionPoseEstimate<DISTORTER>(image_points, points_3d, focal_lengths, principal_points, distortion,
                                             params);
}

template <typename DISTORTER>
boost::optional<PoseWithCovarianceAndInliers> ReprojectionPoseEstimateWithInitialEstimate(
  const std::vector<Eigen::Vector2d>& image_points, const std::vector<Eigen::Vector3d>& points_3d,
  const Eigen::Matrix3d& intrinsics, const Eigen::VectorXd& distortion, const ReprojectionPoseEstimateParams& params,
  const Eigen::Isometry3d& initial_estimate, const std::vector<int>& initial_inliers) {
  const auto focal_lengths = FocalLengths(intrinsics);
  const auto principal_points = PrincipalPoints(intrinsics);
  return ReprojectionPoseEstimateWithInitialEstimate<DISTORTER>(
    image_points, points_3d, focal_lengths, principal_points, distortion, params, initial_estimate, initial_inliers);
}
}  // namespace vision_common
#endif  // VISION_COMMON_POSE_ESTIMATION_H_
