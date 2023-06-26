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
#ifndef DEPTH_ODOMETRY_IMAGE_FEATURES_WITH_KNOWN_CORRESPONDENCES_ALIGNER_DEPTH_ODOMETRY_H_
#define DEPTH_ODOMETRY_IMAGE_FEATURES_WITH_KNOWN_CORRESPONDENCES_ALIGNER_DEPTH_ODOMETRY_H_

#include <depth_odometry/depth_image_features_and_points.h>
#include <depth_odometry/depth_odometry.h>
#include <depth_odometry/image_features_with_known_correspondences_aligner_depth_odometry_params.h>
#include <depth_odometry/point_to_plane_icp_depth_odometry.h>
#include <depth_odometry/pose_with_covariance_and_correspondences.h>
#include <point_cloud_common/point_cloud_with_known_correspondences_aligner.h>
#include <vision_common/feature_detector_and_matcher.h>

namespace depth_odometry {
class ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometry : public DepthOdometry {
 public:
  explicit ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometry(
    const ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams& params);
  boost::optional<PoseWithCovarianceAndCorrespondences> DepthImageCallback(
    const localization_measurements::DepthImageMeasurement& depth_image) final;
  const ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams& params() const { return params_; }

 private:
  bool ValidImagePoint(const Eigen::Vector2d& image_point) const;
  bool Valid3dPoint(const boost::optional<pcl::PointXYZI>& point) const;

  ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams params_;
  point_cloud_common::PointCloudWithKnownCorrespondencesAligner aligner_;
  std::shared_ptr<DepthImageFeaturesAndPoints> previous_depth_image_features_and_points_;
  std::shared_ptr<DepthImageFeaturesAndPoints> latest_depth_image_features_and_points_;
  localization_common::Time previous_timestamp_;
  localization_common::Time latest_timestamp_;
  std::unique_ptr<vision_common::FeatureDetectorAndMatcher> feature_detector_and_matcher_;
  cv::Ptr<cv::CLAHE> clahe_;
  bool normals_required_;
  boost::optional<PointToPlaneICPDepthOdometry> point_to_plane_icp_depth_odometry_;
};
}  // namespace depth_odometry

#endif  // DEPTH_ODOMETRY_IMAGE_FEATURES_WITH_KNOWN_CORRESPONDENCES_ALIGNER_DEPTH_ODOMETRY_H_
