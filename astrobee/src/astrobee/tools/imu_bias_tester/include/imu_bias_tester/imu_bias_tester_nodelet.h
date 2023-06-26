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
#ifndef IMU_BIAS_TESTER_IMU_BIAS_TESTER_NODELET_H_
#define IMU_BIAS_TESTER_IMU_BIAS_TESTER_NODELET_H_

#include <ff_msgs/GraphState.h>
#include <ff_util/ff_nodelet.h>
#include <imu_bias_tester/imu_bias_tester_wrapper.h>

#include <ros/node_handle.h>
#include <ros/publisher.h>
#include <ros/subscriber.h>
#include <sensor_msgs/Imu.h>

#include <boost/optional.hpp>

namespace imu_bias_tester {
class ImuBiasTesterNodelet : public ff_util::FreeFlyerNodelet {
 public:
  ImuBiasTesterNodelet();

 private:
  void Initialize(ros::NodeHandle* nh) final;

  void SubscribeAndAdvertise(ros::NodeHandle* nh);

  void ImuCallback(const sensor_msgs::Imu::ConstPtr& imu_msg);

  void LocalizationStateCallback(const ff_msgs::GraphState::ConstPtr& loc_msg);

  void Run();

  imu_bias_tester::ImuBiasTesterWrapper imu_bias_tester_wrapper_;
  ros::NodeHandle imu_nh_, loc_nh_;
  ros::CallbackQueue imu_queue_, loc_queue_;
  ros::Subscriber imu_sub_, state_sub_;
  ros::Publisher pose_pub_, velocity_pub_;
};
}  // namespace imu_bias_tester

#endif  // IMU_BIAS_TESTER_IMU_BIAS_TESTER_NODELET_H_
