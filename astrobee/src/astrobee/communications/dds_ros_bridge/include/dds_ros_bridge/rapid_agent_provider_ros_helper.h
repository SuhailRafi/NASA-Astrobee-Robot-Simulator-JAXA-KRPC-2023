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

#ifndef DDS_ROS_BRIDGE_RAPID_AGENT_PROVIDER_ROS_HELPER_H_
#define DDS_ROS_BRIDGE_RAPID_AGENT_PROVIDER_ROS_HELPER_H_

#include <string>
#include <vector>

#include "rapidIo/AgentProvider.h"

#include "rapidUtil/RapidHelper.h"

namespace rapid {

/**
 * Extend AgentProvider to provide a publish method with values keys
 */
class AgentProviderRosHelper : public AgentProvider {
 public:
  AgentProviderRosHelper(AgentTopicPairParameters const& params,
                         const std::string& entity_name);

  /**
   * Publish values with current time
   */
  virtual void Publish(const std::vector<rapid::ParameterUnion>& values);
};

}  // end namespace rapid

#endif  // DDS_ROS_BRIDGE_RAPID_AGENT_PROVIDER_ROS_HELPER_H_
