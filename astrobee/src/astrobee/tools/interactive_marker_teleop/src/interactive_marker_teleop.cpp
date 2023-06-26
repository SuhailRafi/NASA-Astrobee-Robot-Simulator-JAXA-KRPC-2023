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

#include <interactive_marker_teleop.h>

#include <string>

namespace vm = visualization_msgs;

InteractiveMarkerTeleop::InteractiveMarkerTeleop(ros::NodeHandle& nh) : nh(nh) {
  // Start tf listener
  tfListener_ = std::shared_ptr<tf2_ros::TransformListener>(new tf2_ros::TransformListener(tfBuffer_));

  // Make the marker
  server_.reset(new interactive_markers::InteractiveMarkerServer("interactive_marker_teleop", "", false));
  ros::Duration(0.1).sleep();
  menu_handler_.insert("Snap to Astrobee",
                       std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));
  menu_handler_.insert("Go to Position",
                       std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));
  menu_handler_.insert("Dock", std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));
  menu_handler_.insert("Undock", std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));
  menu_handler_.insert("Stop", std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));

  // menu_handler_.insert("Add to plan", &InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1);  //
  // TODO(jdekarske) would be cool to add a new marker for each station menu_handler_.insert("Remove from plan",
  // &InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1);

  make6DofMarker(vm::InteractiveMarkerControl::MOVE_ROTATE_3D);
  server_->applyChanges();

  snapMarkerToAstrobee();
  // publish and acknowledge command actions
  cmd_publisher_ = nh.advertise<ff_msgs::CommandStamped>(TOPIC_COMMAND, 10, true);
  // ack_subscriber_ = nh.subscribe(TOPIC_MANAGEMENT_ACK, 10, &AckCallback); // TODO(jdekarske) status goes in the
  // marker text
}

vm::Marker InteractiveMarkerTeleop::makeMarker(const std::string marker_type) {
  vm::Marker marker;

  // make it yellowish and see through
  marker.color.r = 1.0;
  marker.color.g = 1.0;
  marker.color.b = 0.6;
  marker.color.a = 0.8;
  marker.scale.x = 1.0;
  marker.scale.y = 1.0;
  marker.scale.z = 1.0;

  // return a different marker using meshes from astrobee_media
  if (marker_type == "cube") {
    marker.type = vm::Marker::CUBE;
    marker.scale.x = 0.3175;
    marker.scale.y = 0.3175;
    marker.scale.z = 0.3175;
  } else if (marker_type == "body") {
    marker.type = vm::Marker::MESH_RESOURCE;
    marker.mesh_resource = "package://astrobee_freeflyer/meshes/body.dae";
  } else if (marker_type == "pmc_left") {
    marker.type = vm::Marker::MESH_RESOURCE;
    marker.mesh_resource = "package://astrobee_freeflyer/meshes/pmc_skin_.dae";
  } else if (marker_type == "pmc_right") {
    marker.type = vm::Marker::MESH_RESOURCE;
    marker.mesh_resource = "package://astrobee_freeflyer/meshes/pmc_skin_.dae";
    marker.pose.orientation.z = 1.0;
  }

  return marker;
}

vm::InteractiveMarkerControl& InteractiveMarkerTeleop::makeBoxControl(vm::InteractiveMarker& msg) {
  vm::InteractiveMarkerControl control;
  control.always_visible = true;
  control.markers.push_back(makeMarker("body"));
  control.markers.push_back(makeMarker("pmc_left"));
  control.markers.push_back(makeMarker("pmc_right"));
  msg.controls.push_back(control);

  return msg.controls.back();
}

void InteractiveMarkerTeleop::sendMobilityCommand(std::string command, const geometry_msgs::Pose& desired_pose) {
  if (command == ff_msgs::CommandConstants::CMD_NAME_SIMPLE_MOVE6DOF) {
    // Make ros command message to send to the executive. see `simple_move.cc`
    ff_msgs::CommandStamped move_cmd;
    move_cmd.header.stamp = ros::Time::now();
    move_cmd.cmd_name = ff_msgs::CommandConstants::CMD_NAME_SIMPLE_MOVE6DOF;
    move_cmd.cmd_id = "interactive_marker" + std::to_string(move_cmd.header.stamp.sec);
    move_cmd.cmd_src = "interactive_marker";
    move_cmd.subsys_name = "Astrobee";

    // Move command has 4 arguements; frame, xyz, xyz tolerance, and rotation
    move_cmd.args.resize(4);
    move_cmd.args[0].data_type = ff_msgs::CommandArg::DATA_TYPE_STRING;
    move_cmd.args[0].s = "world";

    // Set location where you want Astrobee to go to
    move_cmd.args[1].data_type = ff_msgs::CommandArg::DATA_TYPE_VEC3d;
    move_cmd.args[1].vec3d[0] = desired_pose.position.x;  // x
    move_cmd.args[1].vec3d[1] = desired_pose.position.y;  // y
    move_cmd.args[1].vec3d[2] = desired_pose.position.z;  // z

    // "Tolerance not used!"
    move_cmd.args[2].data_type = ff_msgs::CommandArg::DATA_TYPE_VEC3d;
    move_cmd.args[2].vec3d[0] = 0;
    move_cmd.args[2].vec3d[1] = 0;
    move_cmd.args[2].vec3d[2] = 0;

    // Target attitude, quaternion, only the first 4 values are used
    move_cmd.args[3].data_type = ff_msgs::CommandArg::DATA_TYPE_MAT33f;
    move_cmd.args[3].mat33f[0] = desired_pose.orientation.x;
    move_cmd.args[3].mat33f[1] = desired_pose.orientation.y;
    move_cmd.args[3].mat33f[2] = desired_pose.orientation.z;
    move_cmd.args[3].mat33f[3] = desired_pose.orientation.w;
    move_cmd.args[3].mat33f[4] = 0;
    move_cmd.args[3].mat33f[5] = 0;
    move_cmd.args[3].mat33f[6] = 0;
    move_cmd.args[3].mat33f[7] = 0;
    move_cmd.args[3].mat33f[8] = 0;

    // Send command
    cmd_publisher_.publish(move_cmd);
  } else {
    ROS_ERROR("only simplemove supported with pose");
  }
}

void InteractiveMarkerTeleop::sendMobilityCommand(std::string command) {
  ff_msgs::CommandStamped cmd;
  cmd.header.stamp = ros::Time::now();
  cmd.subsys_name = "Astrobee";
  cmd.cmd_name = command;
  cmd.cmd_id = command;

  if (command == ff_msgs::CommandConstants::CMD_NAME_DOCK) {
    // Dock has one argument
    cmd.args.resize(1);
    cmd.args[0].data_type = ff_msgs::CommandArg::DATA_TYPE_INT;
    cmd.args[0].i = 1;  // TODO(jdekarske) support other berth
  }

  cmd_publisher_.publish(cmd);
}

void InteractiveMarkerTeleop::snapMarkerToAstrobee() {
  // get astrobee position
  geometry_msgs::TransformStamped t;
  t = tfBuffer_.lookupTransform("world", "body", ros::Time(0));
  // set marker pose
  geometry_msgs::Pose p;
  p.position.x = t.transform.translation.x;
  p.position.y = t.transform.translation.y;
  p.position.z = t.transform.translation.z;
  p.orientation = t.transform.rotation;
  server_->setPose("astrobee_teleop", p);
  server_->applyChanges();
}

void InteractiveMarkerTeleop::processFeedback(const vm::InteractiveMarkerFeedbackConstPtr& feedback) {
  // TODO(jdekarske) This is a switch for adding a keepout zone check (see below)
  switch (feedback->event_type) {
    case vm::InteractiveMarkerFeedback::MENU_SELECT:
      switch (feedback->menu_entry_id) {
        case 1:  // snap to astrobee
        {
          snapMarkerToAstrobee();
          break;
        }
        case 2:  // move to desired position
        {
          sendMobilityCommand(ff_msgs::CommandConstants::CMD_NAME_SIMPLE_MOVE6DOF, feedback->pose);
          break;
        }
        case 3:  // dock
        {
          sendMobilityCommand(ff_msgs::CommandConstants::CMD_NAME_DOCK);
          break;
        }
        case 4:  // undock
        {
          sendMobilityCommand(ff_msgs::CommandConstants::CMD_NAME_UNDOCK);
          break;
        }
        case 5:  // stop
        {
          sendMobilityCommand(ff_msgs::CommandConstants::CMD_NAME_STOP_ALL_MOTION);
          break;
        }
        default:
          break;
      }
      break;

      // TODO(jdekarske) check for keep out zones here and turn astrobee marker red if invalid
      // case vm::InteractiveMarkerFeedback::POSE_UPDATE:
      //   ROS_INFO_STREAM(s.str() << ": pose changed"
      //                           << "\nposition = "
      //                           << feedback->pose.position.x
      //                           << ", " << feedback->pose.position.y
      //                           << ", " << feedback->pose.position.z
      //                           << "\norientation = "
      //                           << feedback->pose.orientation.w
      //                           << ", " << feedback->pose.orientation.x
      //                           << ", " << feedback->pose.orientation.y
      //                           << ", " << feedback->pose.orientation.z
      //                           << "\nframe: " << feedback->header.frame_id
      //                           << " time: " << feedback->header.stamp.sec << "sec, "
      //                           << feedback->header.stamp.nsec << " nsec");
      //   break;
      // TODO(jdekarske) check for keep out zones here and turn astrobee marker red if invalid
  }
}

void InteractiveMarkerTeleop::make6DofMarker(unsigned int interaction_mode) {
  vm::InteractiveMarker int_marker;
  int_marker.header.frame_id = "world";
  int_marker.scale = 1;

  int_marker.name = "astrobee_teleop";
  int_marker.description = "Astrobee Command";

  // insert a box
  makeBoxControl(int_marker);
  int_marker.controls[0].interaction_mode = interaction_mode;

  vm::InteractiveMarkerControl control;

  tf2::Quaternion control_orientation(1.0, 0.0, 0.0, 1.0);
  control_orientation.normalize();
  tf2::convert(control_orientation, control.orientation);
  control.name = "rotate_x";
  control.interaction_mode = vm::InteractiveMarkerControl::ROTATE_AXIS;
  int_marker.controls.push_back(control);
  control.name = "move_x";
  control.interaction_mode = vm::InteractiveMarkerControl::MOVE_AXIS;
  int_marker.controls.push_back(control);

  control_orientation = tf2::Quaternion(0.0, 1.0, 0.0, 1.0);
  control_orientation.normalize();
  tf2::convert(control_orientation, control.orientation);
  control.name = "rotate_z";
  control.interaction_mode = vm::InteractiveMarkerControl::ROTATE_AXIS;
  int_marker.controls.push_back(control);
  control.name = "move_z";
  control.interaction_mode = vm::InteractiveMarkerControl::MOVE_AXIS;
  int_marker.controls.push_back(control);

  control_orientation = tf2::Quaternion(0.0, 0.0, 1.0, 1.0);
  control_orientation.normalize();
  tf2::convert(control_orientation, control.orientation);
  control.name = "rotate_y";
  control.interaction_mode = vm::InteractiveMarkerControl::ROTATE_AXIS;
  int_marker.controls.push_back(control);
  control.name = "move_y";
  control.interaction_mode = vm::InteractiveMarkerControl::MOVE_AXIS;
  int_marker.controls.push_back(control);

  server_->insert(int_marker);
  server_->setCallback(int_marker.name,
                       std::bind(&InteractiveMarkerTeleop::processFeedback, this, std::placeholders::_1));
  menu_handler_.apply(*server_, int_marker.name);
}

int main(int argc, char** argv) {
  ros::init(argc, argv, "interactive_marker_teleop");
  ros::NodeHandle nh;

  InteractiveMarkerTeleop teleop(nh);

  ros::spin();

  return 0;
}
