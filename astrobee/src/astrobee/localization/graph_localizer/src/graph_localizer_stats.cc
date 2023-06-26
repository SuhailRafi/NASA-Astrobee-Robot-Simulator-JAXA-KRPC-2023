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
#include <graph_localizer/graph_localizer_stats.h>
#include <graph_localizer/point_to_point_between_factor.h>
#include <graph_localizer/loc_projection_factor.h>
#include <graph_localizer/loc_pose_factor.h>
#include <graph_localizer/pose_rotation_factor.h>
#include <graph_localizer/robust_smart_projection_pose_factor.h>
#include <graph_localizer/utilities.h>
#include <graph_optimizer/utilities.h>

#include <gtsam/geometry/Cal3_S2.h>
#include <gtsam/geometry/PinholePose.h>
#include <gtsam/nonlinear/LinearContainerFactor.h>
#include <gtsam/slam/BetweenFactor.h>
#include <gtsam/slam/ProjectionFactor.h>
#include <gtsam/slam/PriorFactor.h>

namespace graph_localizer {
namespace go = graph_optimizer;
GraphLocalizerStats::GraphLocalizerStats() {
  AddStatsAverager(num_states_averager_);
  AddStatsAverager(duration_averager_);
  AddStatsAverager(num_marginal_factors_averager_);
  AddStatsAverager(num_factors_averager_);
  // AddStatsAverager(num_features_averager_);
  AddStatsAverager(num_depth_odometry_rel_pose_factors_averager_);
  AddStatsAverager(num_depth_odometry_rel_point_factors_averager_);
  AddStatsAverager(num_optical_flow_factors_averager_);
  AddStatsAverager(num_loc_pose_factors_averager_);
  AddStatsAverager(num_loc_proj_factors_averager_);
  AddStatsAverager(num_imu_factors_averager_);
  AddStatsAverager(num_rotation_factors_averager_);
  AddStatsAverager(num_standstill_between_factors_averager_);
  AddStatsAverager(num_vel_prior_factors_averager_);

  AddErrorAverager(depth_odom_rel_pose_error_averager_);
  AddErrorAverager(depth_odom_rel_point_error_averager_);
  AddErrorAverager(of_error_averager_);
  AddErrorAverager(loc_proj_error_averager_);
  AddErrorAverager(loc_pose_error_averager_);
  AddErrorAverager(imu_error_averager_);
  AddErrorAverager(rotation_error_averager_);
  AddErrorAverager(standstill_between_error_averager_);
  AddErrorAverager(pose_prior_error_averager_);
  AddErrorAverager(velocity_prior_error_averager_);
  AddErrorAverager(bias_prior_error_averager_);
}

void GraphLocalizerStats::SetCombinedNavStateGraphValues(
  std::shared_ptr<const CombinedNavStateGraphValues> combined_nav_state_graph_values) {
  combined_nav_state_graph_values_ = std::move(combined_nav_state_graph_values);
}

void GraphLocalizerStats::UpdateErrors(const gtsam::NonlinearFactorGraph& graph_factors) {
  using Calibration = gtsam::Cal3_S2;
  using Camera = gtsam::PinholePose<Calibration>;
  using RobustSmartFactor = gtsam::RobustSmartProjectionPoseFactor<Calibration>;
  using ProjectionFactor = gtsam::GenericProjectionFactor<gtsam::Pose3, gtsam::Point3>;

  double total_error = 0;
  double depth_odom_rel_pose_factor_error = 0;
  double depth_odom_rel_point_factor_error = 0;
  double optical_flow_factor_error = 0;
  double loc_proj_error = 0;
  double loc_pose_error = 0;
  double imu_factor_error = 0;
  double rotation_factor_error = 0;
  double standstill_between_factor_error = 0;
  double pose_prior_error = 0;
  double velocity_prior_error = 0;
  double bias_prior_error = 0;
  for (const auto& factor : graph_factors) {
    const double error = factor->error(combined_nav_state_graph_values_->values());
    total_error += error;
    // TODO(rsoussan): Differentiate between this and standstill between factor
    const auto depth_odom_rel_pose_factor = dynamic_cast<const gtsam::BetweenFactor<gtsam::Pose3>*>(factor.get());
    if (depth_odom_rel_pose_factor) {
      depth_odom_rel_pose_factor_error += error;
    }
    const auto depth_odom_rel_point_factor = dynamic_cast<const gtsam::PointToPointBetweenFactor*>(factor.get());
    if (depth_odom_rel_point_factor) {
      depth_odom_rel_point_factor_error += error;
    }
    const auto smart_factor = dynamic_cast<const RobustSmartFactor*>(factor.get());
    if (smart_factor) {
      optical_flow_factor_error += error;
    }
    const auto projection_factor = dynamic_cast<const ProjectionFactor*>(factor.get());
    if (projection_factor) {
      optical_flow_factor_error += error;
    }
    const auto imu_factor = dynamic_cast<gtsam::CombinedImuFactor*>(factor.get());
    if (imu_factor) {
      imu_factor_error += error;
    }
    const auto loc_factor = dynamic_cast<gtsam::LocProjectionFactor<>*>(factor.get());
    if (loc_factor) {
      loc_proj_error += error;
    }
    const auto loc_pose_factor = dynamic_cast<gtsam::LocPoseFactor*>(factor.get());
    if (loc_pose_factor) {
      loc_pose_error += error;
    }
    const auto rotation_factor = dynamic_cast<gtsam::PoseRotationFactor*>(factor.get());
    if (rotation_factor) {
      rotation_factor_error += error;
    }
    const auto standstill_between_factor = dynamic_cast<gtsam::BetweenFactor<gtsam::Pose3>*>(factor.get());
    if (standstill_between_factor) {
      standstill_between_factor_error += error;
    }

    // Prior Factors
    const auto pose_prior_factor = dynamic_cast<gtsam::PriorFactor<gtsam::Pose3>*>(factor.get());
    if (pose_prior_factor && !loc_pose_factor) {
      pose_prior_error += error;
    }
    const auto velocity_prior_factor = dynamic_cast<gtsam::PriorFactor<gtsam::Velocity3>*>(factor.get());
    if (velocity_prior_factor) {
      velocity_prior_error += error;
    }
    const auto bias_prior_factor = dynamic_cast<gtsam::PriorFactor<gtsam::imuBias::ConstantBias>*>(factor.get());
    if (bias_prior_factor) {
      bias_prior_error += error;
    }
  }
  total_error_averager_.Update(total_error);
  depth_odom_rel_pose_error_averager_.Update(depth_odom_rel_pose_factor_error);
  depth_odom_rel_point_error_averager_.Update(depth_odom_rel_point_factor_error);
  of_error_averager_.Update(optical_flow_factor_error);
  loc_proj_error_averager_.Update(loc_proj_error);
  loc_pose_error_averager_.Update(loc_pose_error);
  imu_error_averager_.Update(imu_factor_error);
  rotation_error_averager_.Update(rotation_factor_error);
  standstill_between_error_averager_.Update(standstill_between_factor_error);
  pose_prior_error_averager_.Update(pose_prior_error);
  velocity_prior_error_averager_.Update(velocity_prior_error);
  bias_prior_error_averager_.Update(bias_prior_error);
}

void GraphLocalizerStats::UpdateStats(const gtsam::NonlinearFactorGraph& graph_factors) {
  num_states_averager_.Update(combined_nav_state_graph_values_->NumStates());
  duration_averager_.Update(combined_nav_state_graph_values_->Duration());
  num_marginal_factors_averager_.Update(go::NumFactors<gtsam::LinearContainerFactor>(graph_factors));
  num_factors_averager_.Update(graph_factors.size());
  num_optical_flow_factors_averager_.Update(
    NumSmartFactors(graph_factors, combined_nav_state_graph_values_->values(), true));
  num_depth_odometry_rel_pose_factors_averager_.Update(
    go::NumFactors<gtsam::BetweenFactor<gtsam::Pose3>>(graph_factors));
  num_depth_odometry_rel_point_factors_averager_.Update(
    go::NumFactors<gtsam::PointToPointBetweenFactor>(graph_factors));
  num_loc_pose_factors_averager_.Update(go::NumFactors<gtsam::LocPoseFactor>(graph_factors));
  num_loc_proj_factors_averager_.Update(go::NumFactors<gtsam::LocProjectionFactor<>>(graph_factors));
  num_imu_factors_averager_.Update(go::NumFactors<gtsam::CombinedImuFactor>(graph_factors));
  num_rotation_factors_averager_.Update(go::NumFactors<gtsam::PoseRotationFactor>(graph_factors));
  num_standstill_between_factors_averager_.Update(go::NumFactors<gtsam::BetweenFactor<gtsam::Pose3>>(graph_factors));
  num_vel_prior_factors_averager_.Update(go::NumFactors<gtsam::PriorFactor<gtsam::Velocity3>>(graph_factors));
}
}  // namespace graph_localizer
