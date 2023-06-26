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


#ifndef LASER_ROS_LASER_NODELET_H_
#define LASER_ROS_LASER_NODELET_H_


#include <pluginlib/class_list_macros.h>

#include <ros/ros.h>
#include <ros/service_server.h>


#include <config_reader/config_reader.h>
#include <ff_hw_msgs/SetEnabled.h>
#include <ff_util/ff_names.h>
#include <ff_util/ff_nodelet.h>

#include <i2c/i2c_new.h>
#include <laser/laser.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>  // NOLINT

namespace laser {
namespace ros {

class LaserNodelet : public ::ff_util::FreeFlyerNodelet {
 public:
  LaserNodelet();
  ~LaserNodelet();
  bool OnService(::ff_hw_msgs::SetEnabled::Request &req,
                 ::ff_hw_msgs::SetEnabled::Response &resp);

 protected:
  virtual void Initialize(::ros::NodeHandle *nh);

 private:
  using LaserPtr = std::unique_ptr<Laser>;

  LaserPtr laser_;
  ::ros::ServiceServer server_;
};

}  // namespace ros
}  // namespace laser

#endif  // LASER_ROS_LASER_NODELET_H_
