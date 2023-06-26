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

#ifndef GRAPH_LOCALIZER_UTILITIES_H_
#define GRAPH_LOCALIZER_UTILITIES_H_

#include <ff_msgs/DepthLandmarks.h>
#include <ff_msgs/GraphState.h>
#include <ff_msgs/LocalizationGraph.h>
#include <ff_msgs/VisualLandmarks.h>
#include <graph_localizer/combined_nav_state_graph_values.h>
#include <graph_localizer/feature_counts.h>
#include <graph_localizer/feature_track.h>
#include <graph_localizer/graph_localizer.h>
#include <graph_localizer/graph_localizer_initializer.h>
#include <graph_localizer/graph_localizer_stats.h>
#include <localization_common/combined_nav_state.h>
#include <localization_common/combined_nav_state_covariances.h>
#include <localization_measurements/feature_point.h>
#include <localization_measurements/imu_measurement.h>
#include <localization_measurements/timestamped_pose.h>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/linear/NoiseModel.h>

#include <Eigen/Core>

#include <boost/optional.hpp>

#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Header.h>

#include <deque>
#include <string>
#include <unordered_set>
#include <vector>

namespace graph_localizer {
bool ValidPointSet(const int num_points, const double average_distance_from_mean,
                   const double min_avg_distance_from_mean, const int min_num_points);

double AverageDistanceFromMean(const std::vector<localization_measurements::FeaturePoint>& points);

bool ValidVLMsg(const ff_msgs::VisualLandmarks& visual_landmarks_msg, const int min_num_landmarks);

bool ValidDepthMsg(const ff_msgs::DepthLandmarks& depth_landmarks_msg);

ff_msgs::GraphState GraphStateMsg(const localization_common::CombinedNavState& combined_nav_state,
                                  const localization_common::CombinedNavStateCovariances& covariances,
                                  const FeatureCounts& detected_feature_counts, const bool estimating_bias,
                                  const double position_log_det_threshold, const double orientation_log_det_threshold,
                                  const bool standstill, const GraphLocalizerStats& graph_stats,
                                  const localization_measurements::FanSpeedMode fan_speed_mode);

ff_msgs::LocalizationGraph GraphMsg(const GraphLocalizer& graph_localizer);

geometry_msgs::PoseStamped PoseMsg(const Eigen::Isometry3d& global_T_body, const std_msgs::Header& header);

geometry_msgs::PoseStamped PoseMsg(const Eigen::Isometry3d& global_T_body, const localization_common::Time time);

geometry_msgs::PoseStamped PoseMsg(const gtsam::Pose3& global_T_body, const localization_common::Time time);

geometry_msgs::PoseStamped PoseMsg(const localization_measurements::TimestampedPose& timestamped_pose);

gtsam::noiseModel::Robust::shared_ptr Robust(const gtsam::SharedNoiseModel& noise, const double huber_k);

boost::optional<SharedRobustSmartFactor> FixSmartFactorByRemovingIndividualMeasurements(
  const SmartProjectionFactorAdderParams& params, const RobustSmartFactor& smart_factor,
  const gtsam::SmartProjectionParams& smart_projection_params, const CombinedNavStateGraphValues& graph_values);

boost::optional<SharedRobustSmartFactor> FixSmartFactorByRemovingMeasurementSequence(
  const SmartProjectionFactorAdderParams& params, const RobustSmartFactor& smart_factor,
  const gtsam::SmartProjectionParams& smart_projection_params, const CombinedNavStateGraphValues& graph_values);

SharedRobustSmartFactor RemoveSmartFactorMeasurements(const RobustSmartFactor& smart_factor,
                                                      const std::unordered_set<int>& factor_key_indices_to_remove,
                                                      const SmartProjectionFactorAdderParams& params,
                                                      const gtsam::SmartProjectionParams& smart_projection_params);

int NumSmartFactors(const gtsam::NonlinearFactorGraph& graph_factors, const gtsam::Values& values,
                    const bool check_valid);
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_UTILITIES_H_
