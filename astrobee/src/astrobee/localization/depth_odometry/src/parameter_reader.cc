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

#include <depth_odometry/parameter_reader.h>
#include <localization_common/logger.h>
#include <msg_conversions/msg_conversions.h>
#include <point_cloud_common/parameter_reader.h>
#include <vision_common/parameter_reader.h>

namespace depth_odometry {
namespace mc = msg_conversions;
namespace pc = point_cloud_common;
namespace vc = vision_common;

void LoadDepthOdometryWrapperParams(config_reader::ConfigReader& config, DepthOdometryWrapperParams& params) {
  params.max_image_and_point_cloud_time_diff = mc::LoadDouble(config, "max_image_and_point_cloud_time_diff");
  params.method = mc::LoadString(config, "depth_odometry_method");
  params.body_T_haz_cam = msg_conversions::LoadEigenTransform(config, "haz_cam_transform");
  params.haz_cam_A_haz_depth = Eigen::Affine3d::Identity();
  LoadPointToPlaneICPDepthOdometryParams(config, params.icp);
  LoadImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams(config, params.image_features);
}

void LoadDepthOdometryParams(config_reader::ConfigReader& config, DepthOdometryParams& params) {
  params.max_time_diff = mc::LoadDouble(config, "max_time_diff");
  params.position_covariance_threshold = mc::LoadDouble(config, "position_covariance_threshold");
  params.orientation_covariance_threshold = mc::LoadDouble(config, "orientation_covariance_threshold");
}

void LoadPointToPlaneICPDepthOdometryParams(config_reader::ConfigReader& config,
                                            PointToPlaneICPDepthOdometryParams& params) {
  pc::LoadPointToPlaneICPParams(config, params.icp);
  params.downsample = mc::LoadBool(config, "downsample");
  params.downsample_leaf_size = mc::LoadDouble(config, "downsample_leaf_size");
  params.use_organized_normal_estimation = mc::LoadBool(config, "use_organized_normal_estimation");
  const std::string normal_estimation_method_name = mc::LoadString(config, "organized_normal_method");
  if (normal_estimation_method_name == "avg_3d_gradient") {
    params.normal_estimation_method =
      pcl::IntegralImageNormalEstimation<pcl::PointXYZI, pcl::Normal>::NormalEstimationMethod::AVERAGE_3D_GRADIENT;
  } else if (normal_estimation_method_name == "covariance") {
    params.normal_estimation_method =
      pcl::IntegralImageNormalEstimation<pcl::PointXYZI, pcl::Normal>::NormalEstimationMethod::COVARIANCE_MATRIX;
  } else if (normal_estimation_method_name == "avg_depth_change") {
    params.normal_estimation_method =
      pcl::IntegralImageNormalEstimation<pcl::PointXYZI, pcl::Normal>::NormalEstimationMethod::AVERAGE_DEPTH_CHANGE;
  } else {
    LogFatal("LoadPointToPlaneICPDepthOdometryParams: Invalid normal estimation method provided.");
  }
  params.normal_smoothing_size = mc::LoadDouble(config, "normal_smoothing_size");
  params.use_depth_dependent_smoothing = mc::LoadBool(config, "use_depth_dependent_smoothing");
  params.max_depth_change_factor = mc::LoadDouble(config, "max_depth_change_factor");
  params.normal_smoothing_size = mc::LoadDouble(config, "normal_smoothing_size");
  params.use_normal_space_sampling = mc::LoadBool(config, "use_normal_space_sampling");
  params.bins_per_axis = mc::LoadInt(config, "bins_per_axis");
  params.num_samples = mc::LoadInt(config, "num_samples");
  LoadDepthOdometryParams(config, params);
}

void LoadImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams(
  config_reader::ConfigReader& config, ImageFeaturesWithKnownCorrespondencesAlignerDepthOdometryParams& params) {
  pc::LoadPointCloudWithKnownCorrespondencesAlignerParams(config, params.aligner);
  vc::LoadBriskFeatureDetectorAndMatcherParams(config, params.brisk_feature_detector_and_matcher);
  vc::LoadLKOpticalFlowFeatureDetectorAndMatcherParams(config, params.lk_optical_flow_feature_detector_and_matcher);
  vc::LoadSurfFeatureDetectorAndMatcherParams(config, params.surf_feature_detector_and_matcher);
  params.detector = mc::LoadString(config, "detector");
  params.use_clahe = mc::LoadBool(config, "use_clahe");
  params.clahe_grid_length = mc::LoadInt(config, "clahe_grid_length");
  params.clahe_clip_limit = mc::LoadDouble(config, "clahe_clip_limit");
  params.min_x_distance_to_border = mc::LoadDouble(config, "min_x_distance_to_border");
  params.min_y_distance_to_border = mc::LoadDouble(config, "min_y_distance_to_border");
  params.min_num_inliers = mc::LoadInt(config, "min_num_inliers");
  params.refine_estimate = mc::LoadBool(config, "refine_estimate");
  if (params.refine_estimate) LoadPointToPlaneICPDepthOdometryParams(config, params.point_to_plane_icp);
  LoadDepthOdometryParams(config, params);
}
}  // namespace depth_odometry
