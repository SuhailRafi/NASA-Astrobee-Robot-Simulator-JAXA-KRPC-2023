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

// ROS includes
#include <ros/ros.h>

// FSW shared libraries
#include <config_reader/config_reader.h>

// FreeFlyer messages
#include <ff_hw_msgs/PmcCommand.h>
#include <ff_hw_msgs/PmcGoal.h>
#include <ff_hw_msgs/PmcTelemetry.h>
#include <ff_hw_msgs/PmcState.h>

// Finite state machine for tracking dock state
#include <ff_util/ff_fsm.h>

// FSW messahes
#include <ff_msgs/SetBool.h>
#include <ff_msgs/FamCommand.h>
#include <ff_msgs/SetFloat.h>

// Autocode inclide
#include <gnc_autocode/blowers.h>

// Gazebo includes
#include <astrobee_gazebo/astrobee_gazebo.h>

// STL includes
#include <string>
#include <map>

namespace gazebo {

using FSM = ff_util::FSM;

// This class is a plugin that calls the GNC autocode to predict
// the forced to be applied to the rigid body
class GazeboModelPluginPmc : public FreeFlyerModelPlugin {
 public:
  static constexpr size_t NUMBER_OF_PMCS      = 2;
  static constexpr size_t NUMBER_OF_NOZZLES   = 6;
  static constexpr double RADS_PER_SEC_TO_RPM = 9.549296585514;

  // All possible states the EPS node can be in
  enum : FSM::State {
    UNKNOWN       = 0,       // We don't know the blower state
    RAMPING_UP    = 1,       // Blowers are ramping up
    RAMPING_DOWN  = 2,       // Blowers are ramping down
    READY         = 3        // Blowers are ready
  };

  // All possible events that can occur
  enum : FSM::Event {
    GOAL_RAMP_UP    = (1<<0),  // Ramp up the PMC
    GOAL_RAMP_DOWN  = (1<<1),  // Ramp down the PMC
    PMC1_READY      = (1<<2),  // We are far from any berth
    PMC2_READY      = (1<<3)   // We received an undock command
  };

  // Constructor
  GazeboModelPluginPmc() : FreeFlyerModelPlugin("pmc_actuator", "", true),
    fsm_(UNKNOWN, std::bind(&GazeboModelPluginPmc::StateCallback, this,
      std::placeholders::_1, std::placeholders::_2)), pmc_enabled_(true),
        bypass_blower_model_(true) {
    // Mark as not ready
    ready_[0] = false;
    ready_[1] = false;
    // Only transition to READY state if both PMC1 and PMC2 are ready
    fsm_.Add(UNKNOWN, RAMPING_UP, RAMPING_DOWN, PMC1_READY | PMC2_READY,
      [this](FSM::Event const& event) -> FSM::State {
        // Update the ready flag based on the event
        switch (event) {
        case PMC1_READY: ready_[0] = true; break;
        case PMC2_READY: ready_[1] = true; break;
        default: break;
        }
        // Only return ready if both PMCs are marked as ready
        if (ready_[0] && ready_[1])
          return READY;
        // Otherwise, return the current state
        return fsm_.GetState();
    });
    // If we are ready and we receive a up or down ramp goal
    fsm_.Add(READY, GOAL_RAMP_UP | GOAL_RAMP_DOWN,
      [this](FSM::Event const& event) -> FSM::State {
        // Reset the ready flags for each P<C
        ready_[0] = false;
        ready_[1] = false;
        // Change state based on the event
        switch (event) {
        case GOAL_RAMP_UP:    return RAMPING_UP;
        case GOAL_RAMP_DOWN:  return RAMPING_DOWN;
        default:
          break;
        }
        // We should never get here
        return UNKNOWN;
    });
  }

  // Called when the state changes
  void StateCallback(FSM::State const& state, FSM::FSM::Event const& event) {
    ff_hw_msgs::PmcState msg;
    msg.header.frame_id = frame_id_;
    msg.header.stamp = ros::Time::now();
    msg.states.resize(NUMBER_OF_PMCS);
    for (size_t i = 0; i < NUMBER_OF_PMCS; i++) {
      switch (state) {
      case UNKNOWN:
        msg.states[i] = ff_hw_msgs::PmcState::UNKNOWN;
        break;
      case RAMPING_UP:
        if (ready_[i])
          msg.states[i] = ff_hw_msgs::PmcState::READY;
        else
          msg.states[i] = ff_hw_msgs::PmcState::RAMPING_UP;
        break;
      case RAMPING_DOWN:
        if (ready_[i])
          msg.states[i] = ff_hw_msgs::PmcState::READY;
        else
          msg.states[i] = ff_hw_msgs::PmcState::RAMPING_DOWN;
        break;
      case READY:
        msg.states[i] = ff_hw_msgs::PmcState::READY;
        break;
      }
    }
    pub_state_.publish(msg);
  }

  // Destructor
  ~GazeboModelPluginPmc() {
    #if GAZEBO_MAJOR_VERSION > 7
    update_.reset();
    #else
    event::Events::DisconnectWorldUpdateBegin(update_);
    #endif
  }

 protected:
  // Called when the plugin is loaded into the simulator
  void LoadCallback(ros::NodeHandle *nh,
    physics::ModelPtr model, sdf::ElementPtr sdf) {
    config_params.AddFile("hw/pmc_actuator.config");
    if (!GetParams()) {
      ROS_ERROR("PMC Actuator: Failed to get parameters.");
      AssertFault(ff_util::INITIALIZATION_FAILED,
                  "Could not get PMC parameters");
    }

    // If we specify a frame name different to our sensor tag name
    if (sdf->HasElement("bypass_blower_model"))
      bypass_blower_model_ = sdf->Get<bool>("bypass_blower_model");

    // Create a null command to be used later
    ff_hw_msgs::PmcGoal null_goal;
    null_goal.motor_speed = 0;
    null_goal.nozzle_positions = {0, 0, 0, 0, 0, 0};
    null_command_.header.frame_id = GetFrame();
    null_command_.goals.push_back(null_goal);
    null_command_.goals.push_back(null_goal);

    // Set timeout
    watchdog_period_ = ros::Duration(20.0/control_rate_hz_);

    // Telemetry publisher
    pub_telemetry_ = nh->advertise<ff_hw_msgs::PmcTelemetry>(
      TOPIC_HARDWARE_PMC_TELEMETRY, 1);

    // State publisher as a latched topic
    pub_state_ = nh->advertise<ff_hw_msgs::PmcState>(
      TOPIC_HARDWARE_PMC_STATE, 1, true);

    // Subscibe to PMC commands
    sub_command_ = nh->subscribe(TOPIC_HARDWARE_PMC_COMMAND, 5,
      &GazeboModelPluginPmc::CommandCallback, this);

    // Now register to be called back every time FAM has new wrench
    if (bypass_blower_model_)
      sub_fam_ = nh->subscribe(TOPIC_GNC_CTL_COMMAND, 5,
        &GazeboModelPluginPmc::FamCallback, this);

    // Create a watchdog timer to ensure the PMC commands are set
    timer_watchdog_ = nh->createTimer(watchdog_period_,
      &GazeboModelPluginPmc::WatchdogCallback, this, false, true);

    // Update PMC watchdog timer timeout
    update_timeout_srv_ = nh->advertiseService(
      SERVICE_HARDWARE_PMC_TIMEOUT, &GazeboModelPluginPmc::IdlingTimeoutService, this);

    // Create a watchdog timer to ensure the PMC commands are set
    timer_command_ = nh->createTimer(ros::Duration(1.0/control_rate_hz_),
      &GazeboModelPluginPmc::CommandTimerCallback, this, false, true);

    // Called before each iteration of simulated world update
    update_ = event::Events::ConnectWorldUpdateBegin(
      std::bind(&GazeboModelPluginPmc::WorldUpdateCallback, this));
  }

  // Read the configuration from the LUA config file
  bool GetParams() {
    // Read all config files
    if (!config_params.ReadFiles()) {
      ROS_FATAL("PMC Actuator: Unable to load lua parameters!");
      return false;
    }
    // get frame id
    if (!config_params.GetStr("frame_id", &frame_id_)) {
      ROS_FATAL("PMC Actuator: frame id not specified!");
      return false;
    }
    // get control rate
    if (!config_params.GetPosReal("control_rate_hz", &control_rate_hz_)) {
      ROS_FATAL("PMC Actuator: control rate not specified!");
      return false;
    }
    // get values used to calculate state
    if (!config_params.GetReal("state_command_scale",
      &state_command_scale_)) {
      ROS_FATAL("PMC Actuator: state_command_scale not specified!");
      return false;
    }
    if (!config_params.GetReal("state_telemetry_scale",
      &state_telemetry_scale_)) {
      ROS_FATAL("PMC Actuator: state_telemetry_scale not specified!");
      return false;
    }
    if (!config_params.GetPosReal("max_timeout", &max_timeout_)) {
      ROS_FATAL("PMC Actuator: minimum control rate not specified!");
      return false;
    }
    // TODO(asymingt) This is being phased out.
    state_tol_rads_per_sec_ = 0.1;
    // Success
    return true;
  }

  // Called on simulation reset
  void Reset() {}

  // This is called whenever the controller has new force/torque to apply
  void FamCallback(ff_msgs::FamCommand const& msg) {
    // Immediately reset the watchdog timer
    timer_watchdog_.stop();
    // ros::getGlobalCallbackQueue()->clear();
    timer_watchdog_.start();
    // Send the command only of he PMCs are enabled
    if (pmc_enabled_)
      SendFamCommand(msg);
    else
      SendCommand(null_command_);
  }

  // This is called whenever the controller has new force/torque to apply
  void CommandCallback(ff_hw_msgs::PmcCommand const& msg) {
    // Immediately reset the watchdog timer
    timer_watchdog_.stop();
    // ros::getGlobalCallbackQueue()->clear();
    timer_watchdog_.start();
    // Send the command only of he PMCs are enabled
    if (pmc_enabled_)
      SendCommand(msg);
    else
      SendCommand(null_command_);
  }

  // If this is *ever* called, it means a FAM command was not received - note
  // that we are using a single-threaded spinner so at most one callback is
  // processed at any time which means that we wont have race conditions
  void WatchdogCallback(ros::TimerEvent const& event) {
    // Immediately reset the watchdog timer
    timer_watchdog_.stop();
    // ros::getGlobalCallbackQueue()->clear();
    timer_watchdog_.start();
    // Update the null command time
    null_command_.header.stamp = ros::Time::now();
    // set the blower speed to zero in case we do not receive messages from FAM
    SendCommand(null_command_);
  }

  // Send a FAM command to the PMCs to those specifi
  void SendFamCommand(ff_msgs::FamCommand const& msg) {
    wrench_ = msg.wrench;
  }

  // Send a PMC command to the PMCs to those specifi
  void SendCommand(ff_hw_msgs::PmcCommand const& msg) {
    // Sanity check
    if (msg.goals.size() != NUMBER_OF_PMCS)
      return;
    // Set the impeller and nozzle values to those given in the message
    for (size_t i = 0; i < NUMBER_OF_PMCS; i++) {
      // Take care of ramping in a more responsible way
      if (blowers_.states_[i].impeller_cmd < msg.goals[i].motor_speed)
        fsm_.Update(GOAL_RAMP_UP);
      if (blowers_.states_[i].impeller_cmd > msg.goals[i].motor_speed)
        fsm_.Update(GOAL_RAMP_DOWN);
      // Set the motor speed
      blowers_.states_[i].impeller_cmd = msg.goals[i].motor_speed;
      // Set the nozzles
      for (size_t j = 0; j < NUMBER_OF_NOZZLES; j++)
        blowers_.states_[i].servo_cmd[j]
          = static_cast <float> (msg.goals[i].nozzle_positions[j]);
    }
  }

  // Must be called at 62.5Hz to satisfy the needs of GNC
  void CommandTimerCallback(ros::TimerEvent const& event) {
    // Step the blower model
    #if GAZEBO_MAJOR_VERSION > 7
    blowers_.SetAngularVelocity(
      GetLink()->RelativeAngularVel().X(),
      GetLink()->RelativeAngularVel().Y(),
      GetLink()->RelativeAngularVel().Z());
    #else
    blowers_.SetAngularVelocity(
      GetLink()->GetRelativeAngularVel().x,
      GetLink()->GetRelativeAngularVel().y,
      GetLink()->GetRelativeAngularVel().z);
    #endif
    blowers_.SetBatteryVoltage(14.0);
    blowers_.Step();
    // Calculate the force and torque on the platform
    #if GAZEBO_MAJOR_VERSION > 7
    force_ = ignition::math::Vector3d(0, 0, 0);
    torque_ = ignition::math::Vector3d(0, 0, 0);
    for (size_t i = 0; i < NUMBER_OF_PMCS; i++) {
      force_ += ignition::math::Vector3d(blowers_.states_[i].force_B[0],
        blowers_.states_[i].force_B[1], blowers_.states_[i].force_B[2]);
      torque_ += ignition::math::Vector3d(blowers_.states_[i].torque_B[0],
        blowers_.states_[i].torque_B[1], blowers_.states_[i].torque_B[2]);
    }
    #else
    force_ = math::Vector3(0, 0, 0);
    torque_ = math::Vector3(0, 0, 0);
    for (size_t i = 0; i < NUMBER_OF_PMCS; i++) {
      force_ += math::Vector3(blowers_.states_[i].force_B[0],
        blowers_.states_[i].force_B[1], blowers_.states_[i].force_B[2]);
      torque_ += math::Vector3(blowers_.states_[i].torque_B[0],
        blowers_.states_[i].torque_B[1], blowers_.states_[i].torque_B[2]);
    }
    #endif
    // Publish telemetry
    PublishTelemetry();
  }

  // Send the commands to the PMCs
  void PublishTelemetry() {
    telemetry_vector_.header.frame_id = GetFrame();
    telemetry_vector_.header.stamp = ros::Time::now();
    telemetry_vector_.statuses.clear();
    // Check if we need to change state based on the motor speed
    for (size_t i = 0; i < NUMBER_OF_PMCS; i++) {
      // Convert and check range
      double rpm = round(blowers_.states_[i].motor_speed * RADS_PER_SEC_TO_RPM);
      if (rpm < 0 || rpm > 255)
        rpm = 0;
      // Populate as much telemetry as possible from the blower model
      ff_hw_msgs::PmcStatus t;
      t.motor_speed = static_cast<uint8_t>(rpm);
      // Send the telemetry
      telemetry_vector_.statuses.push_back(t);
      // Determine the current state based on the different between the
      // commanded and telemetry motor speeds, scaled appropriately
      static double crps, trps;
      crps = state_command_scale_ * blowers_.states_[i].impeller_cmd;
      trps = blowers_.states_[i].motor_speed;   // Comes in rads/sec!
      // ROS_INFO_STREAM("PMC delta C " << crps << ":: T " << trps);
      if (fabs(crps - trps) < state_tol_rads_per_sec_) {
        switch (i) {
        case 0: fsm_.Update(PMC1_READY); break;
        case 1: fsm_.Update(PMC2_READY); break;
        default: break;
        }
      }
    }
    // Publish telemetry
    pub_telemetry_.publish(telemetry_vector_);
  }

  // Enable or disable the PMC
  bool EnableService(ff_msgs::SetBool::Request &req,
                     ff_msgs::SetBool::Response &res) {  // NOLINT
    pmc_enabled_ = req.enable;
    if (pmc_enabled_) {
      ROS_INFO("PMC Enabled.");
    } else {
      ROS_INFO("PMC Disabled.");
    }
    res.success = true;
    return true;
  }

  // Update Minimum Control Frequency (and cutoff time)
  bool IdlingTimeoutService(ff_msgs::SetFloat::Request &req,
                      ff_msgs::SetFloat::Response &res) {  // NOLINT
    double new_timeout = req.data;
    // Check if the new rate is within the safe and default limits
    if (new_timeout <= max_timeout_ && new_timeout >= (20.0/control_rate_hz_)) {
      watchdog_period_ = ros::Duration(new_timeout);
      timer_.setPeriod(watchdog_period_);
      ROS_INFO("PMC idling timeout updated.");
      res.success = true;
    } else {
      ROS_INFO("Selected timeout is not within the safe timeout bounds.");
      res.success = false;
    }
    return true;
  }

  // Called on each sensor update event
  void WorldUpdateCallback() {
    if (bypass_blower_model_) {
      #if GAZEBO_MAJOR_VERSION > 7
      GetLink()->AddRelativeForce(ignition::math::Vector3d(
        wrench_.force.x, wrench_.force.y, wrench_.force.z));
      GetLink()->AddRelativeTorque(ignition::math::Vector3d(
        wrench_.torque.x, wrench_.torque.y, wrench_.torque.z));
      #else
      GetLink()->AddRelativeForce(math::Vector3(
        wrench_.force.x, wrench_.force.y, wrench_.force.z));
      GetLink()->AddRelativeTorque(math::Vector3(
        wrench_.torque.x, wrench_.torque.y, wrench_.torque.z));
      #endif
    } else {
      GetLink()->AddRelativeForce(force_);
      GetLink()->AddRelativeTorque(torque_);
    }
  }

 private:
  ff_util::FSM fsm_;                                // Finite state machine
  bool ready_[NUMBER_OF_PMCS];                      // Ready flags
  config_reader::ConfigReader config_params;        // LUA configuration reader
  ros::Publisher pub_telemetry_, pub_state_;        // State/telemetry pubs
  ros::Subscriber sub_command_;                     // Command subscriber
  ros::Subscriber sub_fam_;                         // Fam command subscriber
  ros::ServiceServer update_timeout_srv_;
  ros::Timer timer_command_, timer_watchdog_;       // Timers
  ros::Duration watchdog_period_;
  #if GAZEBO_MAJOR_VERSION > 7
  ignition::math::Vector3d force_;                  // Current body-frame force
  ignition::math::Vector3d torque_;                 // Current body-frame torque
  #else
  math::Vector3 force_;                             // Current body-frame force
  math::Vector3 torque_;                            // Current body-frame torque
  #endif
  geometry_msgs::Wrench wrench_;                    // Used when bypassing PMC
  gnc_autocode::GncBlowersAutocode blowers_;        // Autocode blower iface
  ff_hw_msgs::PmcCommand null_command_;             // PMC null command
  event::ConnectionPtr update_;                     // Update event from gazeo
  ff_hw_msgs::PmcTelemetry telemetry_vector_;       // Telemetry
  bool pmc_enabled_;                                // Is the PMC enabled?
  bool bypass_blower_model_;                        // Bypass the blower model
  double state_command_scale_;                      // Command scale
  double state_telemetry_scale_;                    // Telemetry scale
  double state_tol_rads_per_sec_;                   // RPM tolerance
  double control_rate_hz_;                          // Control rate
  double max_timeout_;                              // Maximum timeout allowed for PMC
  std::string frame_id_;                            // Frame
};

// Register this plugin with the simulator
GZ_REGISTER_MODEL_PLUGIN(GazeboModelPluginPmc)

}   // namespace gazebo
