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

#include <localization_analysis/graph_localizer_simulator.h>

namespace localization_analysis {
namespace lc = localization_common;
GraphLocalizerSimulator::GraphLocalizerSimulator(const GraphLocalizerSimulatorParams& params,
                                                 const std::string& graph_config_path_prefix)
    : GraphLocalizerWrapper(graph_config_path_prefix), params_(params) {}

void GraphLocalizerSimulator::BufferOpticalFlowMsg(const ff_msgs::Feature2dArray& feature_array_msg) {
  of_msg_buffer_.emplace_back(feature_array_msg);
}

void GraphLocalizerSimulator::BufferDepthOdometryMsg(const ff_msgs::DepthOdometry& depth_odometry_msg) {
  depth_odometry_msg_buffer_.emplace_back(depth_odometry_msg);
}

void GraphLocalizerSimulator::BufferVLVisualLandmarksMsg(const ff_msgs::VisualLandmarks& visual_landmarks_msg) {
  vl_msg_buffer_.emplace_back(visual_landmarks_msg);
}

void GraphLocalizerSimulator::BufferARVisualLandmarksMsg(const ff_msgs::VisualLandmarks& visual_landmarks_msg) {
  ar_msg_buffer_.emplace_back(visual_landmarks_msg);
}

void GraphLocalizerSimulator::BufferImuMsg(const sensor_msgs::Imu& imu_msg) { imu_msg_buffer_.emplace_back(imu_msg); }

void GraphLocalizerSimulator::BufferFlightModeMsg(const ff_msgs::FlightMode& flight_mode_msg) {
  flight_mode_msg_buffer_.emplace_back(flight_mode_msg);
}

bool GraphLocalizerSimulator::AddMeasurementsAndUpdateIfReady(const lc::Time& current_time) {
  // If not initialized, add measurements as these are required for initialization.
  // Otherwise add measurements if enough time has passed since last optimization, simulating
  // optimization delay.
  if (Initialized() && last_update_time_ && (current_time - *last_update_time_) < params_.optimization_time) {
    return false;
  }

  // Add measurements
  // Add Flight Mode msgs before IMU so imu filters can be set
  for (const auto& flight_mode_msg : flight_mode_msg_buffer_) {
    FlightModeCallback(flight_mode_msg);
  }
  flight_mode_msg_buffer_.clear();

  for (const auto& imu_msg : imu_msg_buffer_) {
    ImuCallback(imu_msg);
  }
  imu_msg_buffer_.clear();
  for (const auto& depth_odometry_msg : depth_odometry_msg_buffer_) {
    DepthOdometryCallback(depth_odometry_msg);
  }
  depth_odometry_msg_buffer_.clear();
  for (const auto& of_msg : of_msg_buffer_) {
    OpticalFlowCallback(of_msg);
  }
  of_msg_buffer_.clear();

  for (const auto& vl_msg : vl_msg_buffer_) {
    VLVisualLandmarksCallback(vl_msg);
  }
  vl_msg_buffer_.clear();

  for (const auto& ar_msg : ar_msg_buffer_) {
    ARVisualLandmarksCallback(ar_msg);
  }
  ar_msg_buffer_.clear();

  Update();
  last_update_time_ = current_time;
  return true;
}
}  // namespace localization_analysis
