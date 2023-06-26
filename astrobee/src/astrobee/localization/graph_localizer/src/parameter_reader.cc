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

#include <graph_localizer/parameter_reader.h>
#include <graph_localizer/utilities.h>
#include <graph_optimizer/parameter_reader.h>
#include <imu_integration/utilities.h>
#include <localization_common/utilities.h>
#include <msg_conversions/msg_conversions.h>

namespace graph_localizer {
namespace go = graph_optimizer;
namespace ii = imu_integration;
namespace lc = localization_common;
namespace mc = msg_conversions;

void LoadCalibrationParams(config_reader::ConfigReader& config, CalibrationParams& params) {
  params.body_T_dock_cam = lc::LoadTransform(config, "dock_cam_transform");
  params.body_T_nav_cam = lc::LoadTransform(config, "nav_cam_transform");
  params.body_T_perch_cam = lc::LoadTransform(config, "perch_cam_transform");
  params.world_T_dock = lc::LoadTransform(config, "world_dock_transform");
  params.nav_cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "nav_cam")));
  params.dock_cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "dock_cam")));
}

void LoadFactorParams(config_reader::ConfigReader& config, FactorParams& params) {
  LoadHandrailFactorAdderParams(config, params.handrail_adder);
  LoadDepthOdometryFactorAdderParams(config, params.depth_odometry_adder);
  LoadLocFactorAdderParams(config, params.loc_adder);
  LoadARTagLocFactorAdderParams(config, params.ar_tag_loc_adder);
  LoadRotationFactorAdderParams(config, params.rotation_adder);
  LoadProjectionFactorAdderParams(config, params.projection_adder);
  LoadSmartProjectionFactorAdderParams(config, params.smart_projection_adder);
  LoadStandstillFactorAdderParams(config, params.standstill_adder);
}

void LoadHandrailFactorAdderParams(config_reader::ConfigReader& config, HandrailFactorAdderParams& params) {
  params.enabled = mc::LoadBool(config, "handrail_adder_enabled");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.min_num_line_matches = mc::LoadDouble(config, "handrail_adder_min_num_line_matches");
  params.min_num_plane_matches = mc::LoadDouble(config, "handrail_adder_min_num_plane_matches");
  params.point_to_line_stddev = mc::LoadDouble(config, "handrail_adder_point_to_line_stddev");
  params.point_to_plane_stddev = mc::LoadDouble(config, "handrail_adder_point_to_plane_stddev");
  params.body_T_perch_cam = lc::LoadTransform(config, "perch_cam_transform");
  params.use_silu_for_point_to_line_segment_factor =
    mc::LoadBool(config, "handrail_adder_use_silu_for_point_to_line_segment_factor");
}

void LoadARTagLocFactorAdderParams(config_reader::ConfigReader& config, LocFactorAdderParams& params) {
  params.add_pose_priors = mc::LoadBool(config, "ar_tag_loc_adder_add_pose_priors");
  params.add_projections = mc::LoadBool(config, "ar_tag_loc_adder_add_projections");
  params.enabled = params.add_pose_priors || params.add_projections ? true : false;
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.min_num_matches = mc::LoadInt(config, "ar_tag_loc_adder_min_num_matches");
  params.max_num_factors = mc::LoadInt(config, "ar_tag_loc_adder_max_num_factors");
  params.prior_translation_stddev = mc::LoadDouble(config, "ar_tag_loc_adder_prior_translation_stddev");
  params.prior_quaternion_stddev = mc::LoadDouble(config, "ar_tag_loc_adder_prior_quaternion_stddev");
  params.scale_pose_noise_with_num_landmarks =
    mc::LoadBool(config, "ar_tag_loc_adder_scale_pose_noise_with_num_landmarks");
  params.scale_projection_noise_with_num_landmarks =
    mc::LoadBool(config, "ar_tag_loc_adder_scale_projection_noise_with_num_landmarks");
  params.pose_noise_scale = mc::LoadDouble(config, "ar_tag_loc_adder_pose_noise_scale");
  params.projection_noise_scale = mc::LoadDouble(config, "ar_tag_loc_adder_projection_noise_scale");
  params.max_inlier_weighted_projection_norm =
    mc::LoadDouble(config, "ar_tag_loc_adder_max_inlier_weighted_projection_norm");
  params.weight_projections_with_distance = mc::LoadBool(config, "ar_tag_loc_adder_weight_projections_with_distance");
  params.add_prior_if_projections_fail = mc::LoadBool(config, "ar_tag_loc_adder_add_prior_if_projections_fail");
  params.body_T_cam = lc::LoadTransform(config, "dock_cam_transform");
  params.cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "dock_cam")));
  params.cam_noise = gtsam::noiseModel::Isotropic::Sigma(2, mc::LoadDouble(config, "loc_dock_cam_noise_stddev"));
}

void LoadDepthOdometryFactorAdderParams(config_reader::ConfigReader& config, DepthOdometryFactorAdderParams& params) {
  params.enabled = mc::LoadBool(config, "depth_odometry_adder_enabled");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.noise_scale = mc::LoadDouble(config, "depth_odometry_adder_noise_scale");
  params.use_points_between_factor = mc::LoadBool(config, "depth_odometry_adder_use_points_between_factor");
  params.position_covariance_threshold = mc::LoadDouble(config, "depth_odometry_adder_position_covariance_threshold");
  params.orientation_covariance_threshold =
    mc::LoadDouble(config, "depth_odometry_adder_orientation_covariance_threshold");
  params.body_T_sensor = lc::LoadTransform(config, "haz_cam_transform");
  params.point_to_point_error_threshold = mc::LoadDouble(config, "depth_odometry_adder_point_to_point_error_threshold");
  params.pose_translation_norm_threshold =
    mc::LoadDouble(config, "depth_odometry_adder_pose_translation_norm_threshold");
  params.max_num_points_between_factors = mc::LoadDouble(config, "depth_odometry_adder_max_num_points_between_factors");
}

void LoadLocFactorAdderParams(config_reader::ConfigReader& config, LocFactorAdderParams& params) {
  params.add_pose_priors = mc::LoadBool(config, "loc_adder_add_pose_priors");
  params.add_projections = mc::LoadBool(config, "loc_adder_add_projections");
  params.enabled = params.add_pose_priors || params.add_projections ? true : false;
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.min_num_matches = mc::LoadInt(config, "loc_adder_min_num_matches");
  params.max_num_factors = mc::LoadInt(config, "loc_adder_max_num_factors");
  params.prior_translation_stddev = mc::LoadDouble(config, "loc_adder_prior_translation_stddev");
  params.prior_quaternion_stddev = mc::LoadDouble(config, "loc_adder_prior_quaternion_stddev");
  params.scale_pose_noise_with_num_landmarks = mc::LoadBool(config, "loc_adder_scale_pose_noise_with_num_landmarks");
  params.scale_projection_noise_with_num_landmarks =
    mc::LoadBool(config, "loc_adder_scale_projection_noise_with_num_landmarks");
  params.pose_noise_scale = mc::LoadDouble(config, "loc_adder_pose_noise_scale");
  params.projection_noise_scale = mc::LoadDouble(config, "loc_adder_projection_noise_scale");
  params.max_inlier_weighted_projection_norm = mc::LoadDouble(config, "loc_adder_max_inlier_weighted_projection_norm");
  params.weight_projections_with_distance = mc::LoadBool(config, "loc_adder_weight_projections_with_distance");
  params.add_prior_if_projections_fail = mc::LoadBool(config, "loc_adder_add_prior_if_projections_fail");
  params.body_T_cam = lc::LoadTransform(config, "nav_cam_transform");
  params.cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "nav_cam")));
  params.cam_noise = gtsam::noiseModel::Isotropic::Sigma(2, mc::LoadDouble(config, "loc_nav_cam_noise_stddev"));
}

void LoadRotationFactorAdderParams(config_reader::ConfigReader& config, RotationFactorAdderParams& params) {
  params.enabled = mc::LoadBool(config, "rotation_adder_enabled");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.min_avg_disparity = mc::LoadDouble(config, "rotation_adder_min_avg_disparity");
  params.rotation_stddev = mc::LoadDouble(config, "rotation_adder_rotation_stddev");
  params.max_percent_outliers = mc::LoadDouble(config, "rotation_adder_max_percent_outliers");
  params.body_T_nav_cam = lc::LoadTransform(config, "nav_cam_transform");
  params.nav_cam_intrinsics = lc::LoadCameraIntrinsics(config, "nav_cam");
}

void LoadProjectionFactorAdderParams(config_reader::ConfigReader& config, ProjectionFactorAdderParams& params) {
  params.enabled = mc::LoadBool(config, "projection_adder_enabled");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.enable_EPI = mc::LoadBool(config, "projection_adder_enable_EPI");
  params.landmark_distance_threshold = mc::LoadDouble(config, "projection_adder_landmark_distance_threshold");
  params.dynamic_outlier_rejection_threshold =
    mc::LoadDouble(config, "projection_adder_dynamic_outlier_rejection_threshold");
  params.max_num_features = mc::LoadInt(config, "projection_adder_max_num_features");
  params.min_num_measurements_for_triangulation =
    mc::LoadInt(config, "projection_adder_min_num_measurements_for_triangulation");
  params.add_point_priors = mc::LoadBool(config, "projection_adder_add_point_priors");
  params.point_prior_translation_stddev = mc::LoadDouble(config, "projection_adder_point_prior_translation_stddev");
  params.body_T_cam = lc::LoadTransform(config, "nav_cam_transform");
  params.cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "nav_cam")));
  params.cam_noise =
    gtsam::noiseModel::Isotropic::Sigma(2, mc::LoadDouble(config, "optical_flow_nav_cam_noise_stddev"));
}

void LoadSmartProjectionFactorAdderParams(config_reader::ConfigReader& config,
                                          SmartProjectionFactorAdderParams& params) {
  params.enabled = mc::LoadBool(config, "smart_projection_adder_enabled");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.min_avg_distance_from_mean = mc::LoadDouble(config, "smart_projection_adder_min_avg_distance_from_mean");
  params.enable_EPI = mc::LoadBool(config, "smart_projection_adder_enable_EPI");
  params.landmark_distance_threshold = mc::LoadDouble(config, "smart_projection_adder_landmark_distance_threshold");
  params.dynamic_outlier_rejection_threshold =
    mc::LoadDouble(config, "smart_projection_adder_dynamic_outlier_rejection_threshold");
  params.retriangulation_threshold = mc::LoadDouble(config, "smart_projection_adder_retriangulation_threshold");
  params.verbose_cheirality = mc::LoadBool(config, "smart_projection_adder_verbose_cheirality");
  params.robust = mc::LoadBool(config, "smart_projection_adder_robust");
  params.max_num_factors = mc::LoadInt(config, "smart_projection_adder_max_num_factors");
  params.min_num_points = mc::LoadInt(config, "smart_projection_adder_min_num_points");
  params.max_num_points_per_factor = mc::LoadInt(config, "smart_projection_adder_max_num_points_per_factor");
  params.measurement_spacing = mc::LoadInt(config, "smart_projection_adder_measurement_spacing");
  params.feature_track_min_separation = mc::LoadDouble(config, "smart_projection_adder_feature_track_min_separation");
  params.rotation_only_fallback = mc::LoadBool(config, "smart_projection_adder_rotation_only_fallback");
  params.splitting = mc::LoadBool(config, "smart_projection_adder_splitting");
  params.scale_noise_with_num_points = mc::LoadBool(config, "smart_projection_adder_scale_noise_with_num_points");
  params.noise_scale = mc::LoadDouble(config, "smart_projection_adder_noise_scale");
  params.use_allowed_timestamps = mc::LoadBool(config, "smart_projection_adder_use_allowed_timestamps");
  params.body_T_cam = lc::LoadTransform(config, "nav_cam_transform");
  params.cam_intrinsics.reset(new gtsam::Cal3_S2(lc::LoadCameraIntrinsics(config, "nav_cam")));
  params.cam_noise =
    gtsam::noiseModel::Isotropic::Sigma(2, mc::LoadDouble(config, "optical_flow_nav_cam_noise_stddev"));
}

void LoadStandstillFactorAdderParams(config_reader::ConfigReader& config, StandstillFactorAdderParams& params) {
  params.add_velocity_prior = mc::LoadBool(config, "standstill_adder_add_velocity_prior");
  params.add_pose_between_factor = mc::LoadBool(config, "standstill_adder_add_pose_between_factor");
  params.enabled = params.add_velocity_prior || params.add_pose_between_factor;
  params.prior_velocity_stddev = mc::LoadDouble(config, "standstill_adder_prior_velocity_stddev");
  params.pose_between_factor_translation_stddev =
    mc::LoadDouble(config, "standstill_adder_pose_between_factor_translation_stddev");
  params.pose_between_factor_rotation_stddev =
    mc::LoadDouble(config, "standstill_adder_pose_between_factor_rotation_stddev");
  params.huber_k = mc::LoadDouble(config, "huber_k");
}

void LoadFeatureTrackerParams(config_reader::ConfigReader& config, FeatureTrackerParams& params) {
  params.sliding_window_duration = mc::LoadDouble(config, "feature_tracker_sliding_window_duration");
  params.smart_projection_adder_measurement_spacing = mc::LoadInt(config, "smart_projection_adder_measurement_spacing");
}

void LoadSanityCheckerParams(config_reader::ConfigReader& config, SanityCheckerParams& params) {
  params.num_consecutive_pose_difference_failures_until_insane =
    mc::LoadInt(config, "num_consecutive_pose_difference_failures_until_insane");
  params.max_sane_position_difference = mc::LoadDouble(config, "max_sane_position_difference");
  params.check_pose_difference = mc::LoadBool(config, "check_pose_difference");
  params.check_position_covariance = mc::LoadBool(config, "check_position_covariance");
  params.check_orientation_covariance = mc::LoadBool(config, "check_orientation_covariance");
  params.position_covariance_threshold = mc::LoadDouble(config, "position_covariance_threshold");
  params.orientation_covariance_threshold = mc::LoadDouble(config, "orientation_covariance_threshold");
}

void LoadGraphInitializerParams(config_reader::ConfigReader& config, GraphInitializerParams& params) {
  ii::LoadImuIntegratorParams(config, params);
  params.imu_bias_filename = mc::LoadString(config, "imu_bias_file");
  params.num_bias_estimation_measurements = mc::LoadInt(config, "num_bias_estimation_measurements");
}

void LoadCombinedNavStateNodeUpdaterParams(config_reader::ConfigReader& config,
                                           CombinedNavStateNodeUpdaterParams& params) {
  params.starting_prior_translation_stddev = mc::LoadDouble(config, "starting_prior_translation_stddev");
  params.starting_prior_quaternion_stddev = mc::LoadDouble(config, "starting_prior_quaternion_stddev");
  params.starting_prior_velocity_stddev = mc::LoadDouble(config, "starting_prior_velocity_stddev");
  params.starting_prior_accel_bias_stddev = mc::LoadDouble(config, "starting_prior_accel_bias_stddev");
  params.starting_prior_gyro_bias_stddev = mc::LoadDouble(config, "starting_prior_gyro_bias_stddev");
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.add_priors = mc::LoadBool(config, "add_priors");
  params.threshold_bias_uncertainty = mc::LoadBool(config, "threshold_bias_uncertainty");
  params.accel_bias_stddev_threshold = mc::LoadDouble(config, "accel_bias_stddev_threshold");
  params.gyro_bias_stddev_threshold = mc::LoadDouble(config, "gyro_bias_stddev_threshold");
  LoadCombinedNavStateGraphValuesParams(config, params.graph_values);
}

void LoadCombinedNavStateGraphValuesParams(config_reader::ConfigReader& config,
                                           CombinedNavStateGraphValuesParams& params) {
  params.ideal_duration = mc::LoadDouble(config, "ideal_duration");
  params.min_num_states = mc::LoadInt(config, "min_num_states");
  params.max_num_states = mc::LoadInt(config, "max_num_states");
}

void LoadFeaturePointNodeUpdaterParams(config_reader::ConfigReader& config, FeaturePointNodeUpdaterParams& params) {
  params.huber_k = mc::LoadDouble(config, "huber_k");
}

void LoadHandrailParams(config_reader::ConfigReader& config, HandrailParams& params) {
  params.length = mc::LoadDouble(config, "handrail_length");
  params.distance_to_wall = mc::LoadDouble(config, "handrail_wall_min_gap");
}

void LoadGraphLocalizerParams(config_reader::ConfigReader& config, GraphLocalizerParams& params) {
  LoadCalibrationParams(config, params.calibration);
  LoadCombinedNavStateNodeUpdaterParams(config, params.combined_nav_state_node_updater);
  LoadGraphInitializerParams(config, params.graph_initializer);
  LoadFactorParams(config, params.factor);
  LoadFeaturePointNodeUpdaterParams(config, params.feature_point_node_updater);
  LoadFeatureTrackerParams(config, params.feature_tracker);
  LoadHandrailParams(config, params.handrail);
  go::LoadGraphOptimizerParams(config, params.graph_optimizer);
  params.huber_k = mc::LoadDouble(config, "huber_k");
  params.max_standstill_feature_track_avg_distance_from_mean =
    mc::LoadDouble(config, "max_standstill_feature_track_avg_distance_from_mean");
  params.standstill_min_num_points_per_track = mc::LoadInt(config, "standstill_min_num_points_per_track");
  params.standstill_feature_track_duration = mc::LoadDouble(config, "standstill_feature_track_duration");
  params.estimate_world_T_dock_using_loc = mc::LoadBool(config, "estimate_world_T_dock_using_loc");
}

void LoadGraphLocalizerNodeletParams(config_reader::ConfigReader& config, GraphLocalizerNodeletParams& params) {
  params.loc_adder_min_num_matches = mc::LoadInt(config, "loc_adder_min_num_matches");
  params.ar_tag_loc_adder_min_num_matches = mc::LoadInt(config, "ar_tag_loc_adder_min_num_matches");
  params.max_imu_buffer_size = mc::LoadInt(config, "max_imu_buffer_size");
  params.max_optical_flow_buffer_size = mc::LoadInt(config, "max_optical_flow_buffer_size");
  params.max_vl_buffer_size = mc::LoadInt(config, "max_vl_buffer_size");
  params.max_ar_buffer_size = mc::LoadInt(config, "max_ar_buffer_size");
  params.max_depth_odometry_buffer_size = mc::LoadInt(config, "max_depth_odometry_buffer_size");
  params.max_dl_buffer_size = mc::LoadInt(config, "max_dl_buffer_size");
}
}  // namespace graph_localizer
