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

#include <localization_common/logger.h>
#include <localization_common/utilities.h>
#include <point_cloud_common/utilities.h>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Point3.h>

#include <pcl/features/normal_3d.h>
#include <pcl/features/impl/normal_3d.hpp>
#include <pcl/features/impl/fpfh.hpp>
#include <pcl/kdtree/impl/kdtree_flann.hpp>
#include <pcl/registration/ia_ransac.h>
#include <pcl/search/impl/search.hpp>
#include <pcl/search/impl/kdtree.hpp>

namespace point_cloud_common {
namespace lc = localization_common;

pcl::PointCloud<pcl::FPFHSignature33>::Ptr EstimateHistogramFeatures(
  const pcl::PointCloud<pcl::PointXYZINormal>::Ptr cloud_with_normals) {
  pcl::FPFHEstimation<pcl::PointXYZINormal, pcl::PointXYZINormal, pcl::FPFHSignature33> feature_estimator;
  feature_estimator.setInputCloud(cloud_with_normals);
  feature_estimator.setInputNormals(cloud_with_normals);
  // TODO(rsoussan): Pass in kd tree from normal estimation?
  pcl::search::KdTree<pcl::PointXYZINormal>::Ptr kd_tree(new pcl::search::KdTree<pcl::PointXYZINormal>);
  feature_estimator.setSearchMethod(kd_tree);
  // pcl: IMPORTANT: the radius used here has to be larger than the radius used to estimate the surface normals!!!
  feature_estimator.setRadiusSearch(0.05);  // 0.2??
  pcl::PointCloud<pcl::FPFHSignature33>::Ptr features(new pcl::PointCloud<pcl::FPFHSignature33>());
  feature_estimator.compute(*features);
  return features;
}

Eigen::Matrix4f RansacIA(const pcl::PointCloud<pcl::PointXYZINormal>::Ptr source_cloud,
                         const pcl::PointCloud<pcl::PointXYZINormal>::Ptr target_cloud) {
  const auto source_features = EstimateHistogramFeatures(source_cloud);
  const auto target_features = EstimateHistogramFeatures(target_cloud);

  pcl::SampleConsensusInitialAlignment<pcl::PointXYZINormal, pcl::PointXYZINormal, pcl::FPFHSignature33> sac_ia_aligner;
  sac_ia_aligner.setInputSource(source_cloud);
  sac_ia_aligner.setInputTarget(target_cloud);
  sac_ia_aligner.setSourceFeatures(source_features);
  sac_ia_aligner.setTargetFeatures(target_features);
  sac_ia_aligner.setMaximumIterations(10);
  sac_ia_aligner.setMinSampleDistance(0);
  sac_ia_aligner.setNumberOfSamples(100);
  sac_ia_aligner.setCorrespondenceRandomness(1);
  pcl::PointCloud<pcl::PointXYZINormal>::Ptr result(new pcl::PointCloud<pcl::PointXYZINormal>);
  sac_ia_aligner.align(*result);
  // std::cout  <<"sac has converged:"<<scia.hasConverged()<<"  score: "<<scia.getFitnessScore()<<endl;
  // TODO(rsoussan): make boost optional, set thresholds for ransacia fitness and make sure it converged!
  // TODO(rsoussan): invert this???
  // const Matrix<double, 4, 4> final_transform = sac_ia_aligner.getFinalTransform();
  return sac_ia_aligner.getFinalTransformation();
}

Eigen::Matrix<double, 1, 6> PointToPlaneJacobian(const gtsam::Point3& source_point, const gtsam::Vector3& normal,
                                                 const gtsam::Pose3& target_T_source) {
  gtsam::Matrix H1;
  target_T_source.transformFrom(source_point, H1);
  return normal.transpose() * H1;
}

Eigen::Matrix<double, 3, 6> PointToPointJacobian(const gtsam::Point3& source_point,
                                                 const gtsam::Pose3& target_T_source) {
  gtsam::Matrix H1;
  target_T_source.transformFrom(source_point, H1);
  return H1;
}

Eigen::Isometry3d RelativeTransformUmeyama(const std::vector<Eigen::Vector3d>& source_points,
                                           const std::vector<Eigen::Vector3d>& target_points) {
  const int num_points = static_cast<int>(source_points.size());
  Eigen::Matrix<double, 3, Eigen::Dynamic> source_cloud_matrix(3, num_points);
  Eigen::Matrix<double, 3, Eigen::Dynamic> target_cloud_matrix(3, num_points);
  for (int i = 0; i < num_points; ++i) {
    const auto& source_point = source_points[i];
    source_cloud_matrix(0, i) = source_point.x();
    source_cloud_matrix(1, i) = source_point.y();
    source_cloud_matrix(2, i) = source_point.z();

    const auto& target_point = target_points[i];
    target_cloud_matrix(0, i) = target_point.x();
    target_cloud_matrix(1, i) = target_point.y();
    target_cloud_matrix(2, i) = target_point.z();
  }

  const Eigen::Matrix<double, 4, 4> target_T_source = Eigen::umeyama(source_cloud_matrix, target_cloud_matrix, false);
  return Eigen::Isometry3d(target_T_source.matrix());
}

boost::optional<Eigen::Matrix<double, 6, 6>> PointToPointCovariance(const std::vector<Eigen::Vector3d>& source_points,
                                                                    const Eigen::Isometry3d& target_T_source) {
  std::vector<Eigen::Matrix<double, 3, 6>> jacobians;
  const int num_correspondences = static_cast<int>(source_points.size());
  for (int i = 0; i < num_correspondences; ++i) {
    const Eigen::Matrix<double, 3, 6> jacobian = PointToPointJacobian(source_points[i], lc::GtPose(target_T_source));
    if (!jacobian.allFinite()) continue;
    jacobians.emplace_back(jacobian);
  }
  return lc::LeastSquaresCovariance(jacobians);
}

boost::optional<Eigen::Matrix<double, 6, 6>> PointToPlaneCovariance(const std::vector<Eigen::Vector3d>& source_points,
                                                                    const std::vector<Eigen::Vector3d>& target_normals,
                                                                    const Eigen::Isometry3d& target_T_source) {
  std::vector<Eigen::Matrix<double, 1, 6>> jacobians;
  const int num_correspondences = static_cast<int>(source_points.size());
  for (int i = 0; i < num_correspondences; ++i) {
    const Eigen::Matrix<double, 1, 6> jacobian =
      PointToPlaneJacobian(source_points[i], target_normals[i], lc::GtPose(target_T_source));
    if (!jacobian.allFinite()) continue;
    jacobians.emplace_back(jacobian);
  }
  return lc::LeastSquaresCovariance(jacobians);
}

pcl::PointXYZI Interpolate(const pcl::PointXYZI& point_a, const pcl::PointXYZI& point_b, const double alpha) {
  const double beta = 1.0 - alpha;
  pcl::PointXYZI interpolated_point;
  interpolated_point.x = beta * point_a.x + alpha * point_b.x;
  interpolated_point.y = beta * point_a.y + alpha * point_b.y;
  interpolated_point.z = beta * point_a.z + alpha * point_b.z;
  interpolated_point.intensity = beta * point_a.intensity + alpha * point_b.intensity;
  return interpolated_point;
}

template <>
bool ValidPoint<pcl::PointXYZ>(const pcl::PointXYZ& point) {
  return ValidPointXYZ(point);
}

template <>
bool ValidPoint<pcl::PointXYZI>(const pcl::PointXYZI& point) {
  return ValidPointXYZ(point) && ValidIntensity(point);
}

template <>
bool ValidPoint<pcl::PointNormal>(const pcl::PointNormal& point) {
  return ValidPointXYZ(point) && ValidNormal(point);
}

template <>
bool ValidPoint<pcl::PointXYZINormal>(const pcl::PointXYZINormal& point) {
  return ValidPointXYZ(point) && ValidNormal(point) && ValidIntensity(point);
}
}  // namespace point_cloud_common
