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

#include <graph_localizer/graph_localizer.h>
#include <graph_localizer/graph_localizer_params.h>
#include <graph_localizer/test_utilities.h>
#include <localization_common/logger.h>
#include <localization_common/test_utilities.h>
#include <localization_common/utilities.h>
#include <localization_measurements/imu_measurement.h>

#include <gtsam/inference/Symbol.h>

#include <gtest/gtest.h>

namespace gl = graph_localizer;
namespace lc = localization_common;
namespace lm = localization_measurements;
namespace sym = gtsam::symbol_shorthand;

TEST(CombinedNavStateNodeUpdaterTester, ConstantVelocity) {
  auto params = gl::DefaultGraphLocalizerParams();
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  constexpr double kInitialVelocity = 0.1;
  params.graph_initializer.global_V_body_start = Eigen::Vector3d(kInitialVelocity, 0, 0);
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  lc::Time time = 0.0;
  const Eigen::Vector3d relative_translation = kTimeDiff * params.graph_initializer.global_V_body_start;
  Eigen::Isometry3d current_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  // Add initial zero acceleration value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement initial_zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
  graph_localizer.AddImuMeasurement(initial_zero_imu_measurement);
  for (int i = 0; i < kNumIterations; ++i) {
    time += kTimeDiff;
    const lm::ImuMeasurement zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
    graph_localizer.AddImuMeasurement(zero_imu_measurement);
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, Eigen::Matrix3d::Identity());
    current_pose = current_pose * relative_pose;
    const lc::Time source_time = time - kTimeDiff;
    const lc::Time target_time = time;
    const lm::DepthOdometryMeasurement constant_velocity_measurement =
      gl::DepthOdometryMeasurementFromPose(relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_velocity_measurement);
    graph_localizer.Update();
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), current_pose, 1e-5);
  }
}

TEST(CombinedNavStateNodeUpdaterTester, ConstantAcceleration) {
  auto params = gl::DefaultGraphLocalizerParams();
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  lc::Time time = 0.0;
  Eigen::Isometry3d current_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  const Eigen::Vector3d acceleration(0.01, 0.02, 0.03);
  Eigen::Vector3d velocity = params.graph_initializer.global_V_body_start;
  // Add initial zero acceleration value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
  graph_localizer.AddImuMeasurement(zero_imu_measurement);
  for (int i = 0; i < kNumIterations; ++i) {
    time += kTimeDiff;
    const lm::ImuMeasurement imu_measurement(acceleration, Eigen::Vector3d::Zero(), time);
    graph_localizer.AddImuMeasurement(imu_measurement);
    const Eigen::Vector3d relative_translation = velocity * kTimeDiff + 0.5 * acceleration * kTimeDiff * kTimeDiff;
    velocity += acceleration * kTimeDiff;
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, Eigen::Matrix3d::Identity());
    current_pose = current_pose * relative_pose;
    const lc::Time source_time = time - kTimeDiff;
    const lc::Time target_time = time;
    const lm::DepthOdometryMeasurement constant_acceleration_measurement =
      gl::DepthOdometryMeasurementFromPose(relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_acceleration_measurement);
    graph_localizer.Update();
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), current_pose, 1e-5);
  }
}

TEST(CombinedNavStateNodeUpdaterTester, ConstantAccelerationNonZeroBias) {
  const Eigen::Vector3d acceleration(0.01, 0.02, 0.03);
  const Eigen::Vector3d angular_velocity(0.04, 0.05, 0.06);
  const Eigen::Vector3d acceleration_bias = 0.5 * acceleration;
  const Eigen::Vector3d angular_velocity_bias = angular_velocity;
  const Eigen::Vector3d bias_corrected_acceleration = acceleration - acceleration_bias;
  const Eigen::Vector3d bias_corrected_angular_velocity = angular_velocity - angular_velocity_bias;
  auto params = gl::DefaultGraphLocalizerParams();
  params.graph_initializer.initial_imu_bias = gtsam::imuBias::ConstantBias(acceleration_bias, angular_velocity_bias);
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  lc::Time time = 0.0;
  Eigen::Isometry3d current_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  Eigen::Vector3d velocity = params.graph_initializer.global_V_body_start;
  // Add initial zero imu value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement zero_imu_measurement(acceleration_bias, angular_velocity_bias, time);
  graph_localizer.AddImuMeasurement(zero_imu_measurement);
  for (int i = 0; i < kNumIterations; ++i) {
    time += kTimeDiff;
    const lm::ImuMeasurement imu_measurement(acceleration, angular_velocity, time);
    graph_localizer.AddImuMeasurement(imu_measurement);
    const Eigen::Vector3d relative_translation =
      velocity * kTimeDiff + 0.5 * bias_corrected_acceleration * kTimeDiff * kTimeDiff;
    velocity += bias_corrected_acceleration * kTimeDiff;
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, Eigen::Matrix3d::Identity());
    current_pose = current_pose * relative_pose;
    const lc::Time source_time = time - kTimeDiff;
    const lc::Time target_time = time;
    const lm::DepthOdometryMeasurement constant_acceleration_measurement =
      gl::DepthOdometryMeasurementFromPose(relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_acceleration_measurement);
    graph_localizer.Update();
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), current_pose, 1e-5);
  }
}

TEST(CombinedNavStateNodeUpdaterTester, ConstantAccelerationConstantAngularVelocityNonZeroBias) {
  const Eigen::Vector3d acceleration(0.01, 0.02, 0.03);
  const Eigen::Vector3d angular_velocity(0.04, 0.05, 0.06);
  const Eigen::Vector3d acceleration_bias = 0.5 * acceleration;
  const Eigen::Vector3d angular_velocity_bias = 0.5 * angular_velocity;
  const Eigen::Vector3d bias_corrected_acceleration = acceleration - acceleration_bias;
  const Eigen::Vector3d bias_corrected_angular_velocity = angular_velocity - angular_velocity_bias;
  auto params = gl::DefaultGraphLocalizerParams();
  params.graph_initializer.initial_imu_bias = gtsam::imuBias::ConstantBias(acceleration_bias, angular_velocity_bias);
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  lc::Time time = 0.0;
  Eigen::Isometry3d current_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  Eigen::Vector3d velocity = params.graph_initializer.global_V_body_start;
  // Add initial zero imu value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement zero_imu_measurement(acceleration_bias, angular_velocity_bias, time);
  graph_localizer.AddImuMeasurement(zero_imu_measurement);
  for (int i = 0; i < kNumIterations; ++i) {
    time += kTimeDiff;
    const lm::ImuMeasurement imu_measurement(acceleration, angular_velocity, time);
    graph_localizer.AddImuMeasurement(imu_measurement);
    const Eigen::Matrix3d relative_orientation =
      (gtsam::Rot3::Expmap(bias_corrected_angular_velocity * kTimeDiff)).matrix();
    const Eigen::Vector3d relative_translation =
      velocity * kTimeDiff + 0.5 * bias_corrected_acceleration * kTimeDiff * kTimeDiff;
    velocity += bias_corrected_acceleration * kTimeDiff;
    // Put velocity in new body frame after integrating accelerations
    velocity = relative_orientation.transpose() * velocity;
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, relative_orientation);
    current_pose = current_pose * relative_pose;
    const lc::Time source_time = time - kTimeDiff;
    const lc::Time target_time = time;
    const lm::DepthOdometryMeasurement constant_acceleration_measurement =
      gl::DepthOdometryMeasurementFromPose(relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_acceleration_measurement);
    graph_localizer.Update();
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), current_pose, 1e-5);
  }
}

TEST(CombinedNavStateNodeUpdaterTester, SlidingWindow) {
  auto params = gl::DefaultGraphLocalizerParams();
  params.combined_nav_state_node_updater.graph_values.max_num_states = 1000000;
  // Add 1e-5 to avoid float comparison issues when sliding window.
  // In practice, latest_timestamp - ideal_duration shouldn't be identical to a node timestamp
  // and these float comparisons aren't an issue.  Only an issue in tests.
  const double ideal_duration = 3.0;
  params.combined_nav_state_node_updater.graph_values.ideal_duration = ideal_duration + 1e-5;
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  constexpr double kInitialVelocity = 0.1;
  params.graph_initializer.global_V_body_start = Eigen::Vector3d(kInitialVelocity, 0, 0);
  gl::GraphLocalizer graph_localizer(params);
  constexpr int kNumIterations = 100;
  constexpr double kTimeDiff = 0.1;
  // Add 1 for initial state
  const int max_num_states_in_sliding_window = ideal_duration / kTimeDiff + 1;
  lc::Time time = 0.0;
  const Eigen::Vector3d relative_translation = kTimeDiff * params.graph_initializer.global_V_body_start;
  Eigen::Isometry3d current_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  // Add initial zero acceleration value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement initial_zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
  graph_localizer.AddImuMeasurement(initial_zero_imu_measurement);
  lc::Time last_time = 0;
  for (int i = 0; i < kNumIterations; ++i) {
    last_time = time;
    time += kTimeDiff;
    const lm::ImuMeasurement zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), time);
    graph_localizer.AddImuMeasurement(zero_imu_measurement);
    const Eigen::Isometry3d relative_pose = lc::Isometry3d(relative_translation, Eigen::Matrix3d::Identity());
    current_pose = current_pose * relative_pose;
    const lc::Time source_time = last_time;
    const lc::Time target_time = time;
    const lm::DepthOdometryMeasurement constant_velocity_measurement =
      gl::DepthOdometryMeasurementFromPose(relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_velocity_measurement);
    graph_localizer.Update();
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), current_pose, 1e-5);
    // Check num states, ensure window is sliding properly
    // i + 2 since graph is initialized with a starting state and i = 0 also adds a state
    const int total_states_added = i + 2;
    EXPECT_EQ(graph_localizer.combined_nav_state_graph_values().NumStates(),
              std::min(total_states_added, max_num_states_in_sliding_window));
    const auto& combined_nav_state_node_updater = graph_localizer.combined_nav_state_node_updater();
    const lc::Time expected_oldest_timestamp = std::max(0.0, time - ideal_duration);
    // Check latest and oldest timestamps
    {
      const auto oldest_timestamp = combined_nav_state_node_updater.OldestTimestamp();
      ASSERT_TRUE(oldest_timestamp != boost::none);
      EXPECT_NEAR(*oldest_timestamp, expected_oldest_timestamp, 1e-6);
      const auto latest_timestamp = combined_nav_state_node_updater.LatestTimestamp();
      ASSERT_TRUE(latest_timestamp != boost::none);
      EXPECT_NEAR(*latest_timestamp, time, 1e-6);
    }
    // Check node timestamps
    {
      const auto timestamps = graph_localizer.combined_nav_state_graph_values().Timestamps();
      lc::Time expected_timestamp = expected_oldest_timestamp;
      for (const auto timestamp : timestamps) {
        EXPECT_NEAR(timestamp, expected_timestamp, 1e-6);
        expected_timestamp += kTimeDiff;
      }
    }
    // Check corect factors are in graph
    {
      // i + 1 factors for each relative pose factor and imu factor and 3 prior factors for pose, velocity, and bias
      const int num_relative_factors = std::min(30, i + 1);
      const int num_prior_factors = 3;
      // Max 30 relative pose and imu factors due to sliding graph size
      EXPECT_EQ(graph_localizer.num_factors(), 2 * num_relative_factors + num_prior_factors);
      const auto imu_factors = graph_localizer.Factors<gtsam::CombinedImuFactor>();
      EXPECT_EQ(imu_factors.size(), num_relative_factors);
      const auto rel_pose_factors = graph_localizer.Factors<gtsam::BetweenFactor<gtsam::Pose3>>();
      EXPECT_EQ(rel_pose_factors.size(), num_relative_factors);
      // Check priors
      const auto oldest_key_index = graph_localizer.combined_nav_state_graph_values().OldestCombinedNavStateKeyIndex();
      ASSERT_TRUE(oldest_key_index != boost::none);
      const auto pose_prior_factors = graph_localizer.Factors<gtsam::PriorFactor<gtsam::Pose3>>();
      ASSERT_EQ(pose_prior_factors.size(), 1);
      EXPECT_EQ(sym::P(*oldest_key_index), pose_prior_factors[0]->keys()[0]);
      const auto velocity_prior_factors = graph_localizer.Factors<gtsam::PriorFactor<gtsam::Velocity3>>();
      ASSERT_EQ(velocity_prior_factors.size(), 1);
      EXPECT_EQ(sym::V(*oldest_key_index), velocity_prior_factors[0]->keys()[0]);
      const auto imu_bias_prior_factors = graph_localizer.Factors<gtsam::PriorFactor<gtsam::imuBias::ConstantBias>>();
      ASSERT_EQ(imu_bias_prior_factors.size(), 1);
      EXPECT_EQ(sym::B(*oldest_key_index), imu_bias_prior_factors[0]->keys()[0]);
    }
  }
}

TEST(CombinedNavStateNodeUpdaterTester, SplitExistingFactor) {
  auto params = gl::DefaultGraphLocalizerParams();
  params.combined_nav_state_node_updater.graph_values.max_num_states = 1000000;
  constexpr double kInitialVelocity = 0.1;
  params.graph_initializer.global_V_body_start = Eigen::Vector3d(kInitialVelocity, 0, 0);
  // Use depth odometry factor adder since it can add relative pose factors
  params.factor.depth_odometry_adder = gl::DefaultDepthOdometryFactorAdderParams();
  gl::GraphLocalizer graph_localizer(params);
  const auto& combined_nav_state_node_updater = graph_localizer.combined_nav_state_node_updater();
  constexpr double kTimeDiff = 0.1;
  const lc::Time oldest_time = 0.0;
  Eigen::Isometry3d initial_pose = lc::EigenPose(params.graph_initializer.global_T_body_start);
  // Add initial zero acceleration value so the imu integrator has more than one measurement when the subsequent
  // measurement is added
  const lm::ImuMeasurement initial_zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), oldest_time);
  graph_localizer.AddImuMeasurement(initial_zero_imu_measurement);
  const lc::Time latest_time = kTimeDiff;
  const Eigen::Vector3d latest_relative_translation = latest_time * params.graph_initializer.global_V_body_start;
  const Eigen::Isometry3d latest_relative_pose =
    lc::Isometry3d(latest_relative_translation, Eigen::Matrix3d::Identity());
  const Eigen::Isometry3d latest_pose = initial_pose * latest_relative_pose;
  // Add latest measurement
  {
    const lm::ImuMeasurement zero_imu_measurement(Eigen::Vector3d::Zero(), Eigen::Vector3d::Zero(), latest_time);
    graph_localizer.AddImuMeasurement(zero_imu_measurement);
    const lc::Time source_time = oldest_time;
    const lc::Time target_time = latest_time;
    const lm::DepthOdometryMeasurement constant_velocity_measurement =
      gl::DepthOdometryMeasurementFromPose(latest_relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_velocity_measurement);
    graph_localizer.Update();
    EXPECT_EQ(graph_localizer.combined_nav_state_graph_values().NumStates(), 2);
    const auto oldest_timestamp = combined_nav_state_node_updater.OldestTimestamp();
    ASSERT_TRUE(oldest_timestamp != boost::none);
    EXPECT_NEAR(*oldest_timestamp, oldest_time, 1e-6);
    const auto latest_timestamp = combined_nav_state_node_updater.LatestTimestamp();
    ASSERT_TRUE(latest_timestamp != boost::none);
    EXPECT_NEAR(*latest_timestamp, latest_time, 1e-6);
    const auto imu_factors = graph_localizer.Factors<gtsam::CombinedImuFactor>();
    ASSERT_EQ(imu_factors.size(), 1);
    const auto pose_key_1 = imu_factors[0]->key1();
    const auto key_1_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_1);
    ASSERT_TRUE(key_1_time != boost::none);
    EXPECT_NEAR(*key_1_time, oldest_time, 1e-6);
    const auto pose_key_2 = imu_factors[0]->key3();
    const auto key_2_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_2);
    ASSERT_TRUE(key_2_time != boost::none);
    EXPECT_NEAR(*key_2_time, latest_time, 1e-6);
    const auto latest_combined_nav_state = graph_localizer.LatestCombinedNavState();
    ASSERT_TRUE(latest_combined_nav_state != boost::none);
    EXPECT_NEAR(latest_combined_nav_state->timestamp(), latest_time, 1e-6);
    EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), latest_pose, 1e-5);
  }
  // Add measurement between initial and latest times
  {
    const lc::Time middle_time = kTimeDiff / 2.0;
    const Eigen::Vector3d middle_relative_translation = middle_time * params.graph_initializer.global_V_body_start;
    const Eigen::Isometry3d middle_relative_pose =
      lc::Isometry3d(middle_relative_translation, Eigen::Matrix3d::Identity());
    const Eigen::Isometry3d middle_pose = initial_pose * middle_relative_pose;
    const lc::Time source_time = oldest_time;
    const lc::Time target_time = middle_time;
    const lm::DepthOdometryMeasurement constant_velocity_measurement =
      gl::DepthOdometryMeasurementFromPose(middle_relative_pose, source_time, target_time);
    graph_localizer.AddDepthOdometryMeasurement(constant_velocity_measurement);
    graph_localizer.Update();
    EXPECT_EQ(graph_localizer.combined_nav_state_graph_values().NumStates(), 3);
    const auto oldest_timestamp = combined_nav_state_node_updater.OldestTimestamp();
    ASSERT_TRUE(oldest_timestamp != boost::none);
    EXPECT_NEAR(*oldest_timestamp, oldest_time, 1e-6);
    const auto latest_timestamp = combined_nav_state_node_updater.LatestTimestamp();
    ASSERT_TRUE(latest_timestamp != boost::none);
    EXPECT_NEAR(*latest_timestamp, latest_time, 1e-6);
    const auto imu_factors = graph_localizer.Factors<gtsam::CombinedImuFactor>();
    ASSERT_EQ(imu_factors.size(), 2);
    // Check middle imu factor
    {
      const auto& middle_imu_factor = imu_factors[0];
      const auto pose_key_1 = middle_imu_factor->key1();
      const auto key_1_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_1);
      ASSERT_TRUE(key_1_time != boost::none);
      EXPECT_NEAR(*key_1_time, oldest_time, 1e-6);
      const auto pose_key_2 = middle_imu_factor->key3();
      const auto key_2_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_2);
      ASSERT_TRUE(key_2_time != boost::none);
      EXPECT_NEAR(*key_2_time, middle_time, 1e-6);
      const auto middle_combined_nav_state = graph_localizer.GetCombinedNavState(middle_time);
      ASSERT_TRUE(middle_combined_nav_state != boost::none);
      EXPECT_NEAR(middle_combined_nav_state->timestamp(), middle_time, 1e-6);
      EXPECT_MATRIX_NEAR(middle_combined_nav_state->pose(), middle_pose, 1e-5);
    }
    // Check latest imu factor
    {
      const auto& latest_imu_factor = imu_factors[1];
      const auto pose_key_1 = latest_imu_factor->key1();
      const auto key_1_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_1);
      ASSERT_TRUE(key_1_time != boost::none);
      EXPECT_NEAR(*key_1_time, middle_time, 1e-6);
      const auto pose_key_2 = latest_imu_factor->key3();
      const auto key_2_time = graph_localizer.combined_nav_state_graph_values().Timestamp(sym::P, pose_key_2);
      ASSERT_TRUE(key_2_time != boost::none);
      EXPECT_NEAR(*key_2_time, latest_time, 1e-6);
      const auto latest_combined_nav_state = graph_localizer.GetCombinedNavState(latest_time);
      ASSERT_TRUE(latest_combined_nav_state != boost::none);
      EXPECT_NEAR(latest_combined_nav_state->timestamp(), latest_time, 1e-6);
      EXPECT_MATRIX_NEAR(latest_combined_nav_state->pose(), latest_pose, 1e-5);
    }
  }
}

// Run all the tests that were declared with TEST()
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
