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

#ifndef EXECUTIVE_OP_STATE_TELEOP_H_
#define EXECUTIVE_OP_STATE_TELEOP_H_

#include "executive/op_state.h"
#include "executive/op_state_repo.h"
#include <string>

namespace executive {
class OpStateTeleop : public OpState {
 public:
  ~OpStateTeleop() {}

  OpState* HandleCmd(ff_msgs::CommandStampedPtr const& cmd);

  OpState* HandleResult(ff_util::FreeFlyerActionState::Enum const& state,
                        std::string const& result_response,
                        std::string const& cmd_id,
                        Action const& action);

 protected:
  explicit OpStateTeleop(std::string const& name, unsigned char id) :
    OpState(name, id) {}

 private:
  // allow creation only by repo
  friend class OpStateRepo;
  ff_msgs::CommandStampedPtr move_cmd_;
};
}  // namespace executive
#endif  // EXECUTIVE_OP_STATE_TELEOP_H_
