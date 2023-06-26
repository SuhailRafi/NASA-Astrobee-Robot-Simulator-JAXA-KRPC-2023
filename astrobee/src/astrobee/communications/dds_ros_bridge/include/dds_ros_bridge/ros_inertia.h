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

#ifndef DDS_ROS_BRIDGE_ROS_INERTIA_H_
#define DDS_ROS_BRIDGE_ROS_INERTIA_H_

#include <string>

#include "dds_ros_bridge/ros_sub_rapid_pub.h"
#include "dds_ros_bridge/util.h"

#include "geometry_msgs/InertiaStamped.h"

#include "knDds/DdsTypedSupplier.h"

#include "rapidUtil/RapidHelper.h"

#include "dds_msgs/AstrobeeConstants.h"
#include "dds_msgs/InertialPropertiesSupport.h"

namespace ff {

class RosInertiaToRapid : public RosSubRapidPub {
 public:
  RosInertiaToRapid(const std::string& subscribe_topic,
                    const std::string& pub_topic,
                    const ros::NodeHandle &nh,
                    const unsigned int queue_size = 10);

  void InertiaCallback(geometry_msgs::InertiaStampedConstPtr const& inertia);

 private:
  using DataSupplier =
    kn::DdsTypedSupplier<rapid::ext::astrobee::InertialProperties>;
  using DataSupplierPtr = std::unique_ptr<DataSupplier>;

  DataSupplierPtr data_supplier_;
};

}  // end namespace ff

#endif  // DDS_ROS_BRIDGE_ROS_INERTIA_H_
