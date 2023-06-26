#!/usr/bin/python
#
# Copyright (c) 2017, United States Government, as represented by the
# Administrator of the National Aeronautics and Space Administration.
#
# All rights reserved.
#
# The Astrobee platform is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
"""
Plots the localization information from a bagfile to a pdf. Generates 
plots for localization poses, velocities, covariances, integrated velocities, 
IMU bias values, IMU bias tester poses, feature counts, and more.
"""

import argparse
import os
import sys

import loc_states
import matplotlib
import plot_helpers
import poses
import rmse_utilities
import utilities
import vector3d_plotter
import velocities

matplotlib.use("pdf")
import csv
import math

import geometry_msgs
import matplotlib.pyplot as plt
import rosbag
from matplotlib.backends.backend_pdf import PdfPages


def l2_map(vector3ds):
    return [
        math.sqrt(x_y_z[0] * x_y_z[0] + x_y_z[1] * x_y_z[1] + x_y_z[2] * x_y_z[2])
        for x_y_z in zip(vector3ds.xs, vector3ds.ys, vector3ds.zs)
    ]


def add_graph_plots(
    pdf,
    sparse_mapping_poses,
    ar_tag_poses,
    graph_localization_states,
    imu_augmented_graph_localization_poses,
):
    colors = ["r", "b", "g"]
    position_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)", "Position (m)", "Graph vs. Sparse Mapping Position", True
    )
    position_plotter.add_pose_position(
        sparse_mapping_poses,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    if ar_tag_poses.times:
        position_plotter.add_pose_position(
            ar_tag_poses,
            linestyle="None",
            marker="x",
            markeredgewidth=0.1,
            markersize=1.5,
        )
    position_plotter.add_pose_position(graph_localization_states)
    position_plotter.plot(pdf)

    # orientations
    orientation_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)", "Orientation (deg)", "Graph vs. Sparse Mapping Orientation", True
    )
    orientation_plotter.add_pose_orientation(
        sparse_mapping_poses,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    if ar_tag_poses.times:
        orientation_plotter.add_pose_orientation(
            ar_tag_poses,
            linestyle="None",
            marker="x",
            markeredgewidth=0.1,
            markersize=1.5,
        )
    orientation_plotter.add_pose_orientation(graph_localization_states)
    orientation_plotter.plot(pdf)

    # Imu Augmented Loc vs. Loc
    position_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)", "Position (m)", "Graph vs. IMU Augmented Graph Position", True
    )
    position_plotter.add_pose_position(
        graph_localization_states,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )

    position_plotter.add_pose_position(
        imu_augmented_graph_localization_poses, linewidth=0.5
    )
    position_plotter.plot(pdf)

    # orientations
    orientation_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)",
        "Orientation (deg)",
        "Graph vs. IMU Augmented Graph Orientation",
        True,
    )
    orientation_plotter.add_pose_orientation(
        graph_localization_states, marker="o", markeredgewidth=0.1, markersize=1.5
    )
    orientation_plotter.add_pose_orientation(
        imu_augmented_graph_localization_poses, linewidth=0.5
    )
    orientation_plotter.plot(pdf)

    # Velocity
    plt.figure()
    plot_helpers.plot_vector3ds(
        graph_localization_states.velocities, graph_localization_states.times, "Vel."
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Velocities")
    plt.title("Graph Velocities")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Integrated Velocities
    integrated_graph_localization_states = utilities.integrate_velocities(
        graph_localization_states
    )
    plot_positions(
        pdf, integrated_graph_localization_states, sparse_mapping_poses, ar_tag_poses
    )


# TODO(rsoussan): Use this for other pose plotting in this script
def plot_poses(pdf, poses, sparse_mapping_poses, ar_tag_poses):
    plot_positions(pdf, poses, sparse_mapping_poses, ar_tag_poses)
    plot_orientations(pdf, poses, sparse_mapping_poses, ar_tag_poses)


def plot_positions(pdf, position_poses, sparse_mapping_poses, ar_tag_poses):
    position_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)",
        "Position (m)",
        position_poses.pose_type + " vs. Sparse Mapping Position",
        True,
    )
    position_plotter.add_pose_position(
        sparse_mapping_poses,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    if ar_tag_poses.times:
        position_plotter.add_pose_position(
            ar_tag_poses,
            linestyle="None",
            marker="x",
            markeredgewidth=0.1,
            markersize=1.5,
        )
    position_plotter.add_pose_position(position_poses)
    position_plotter.plot(pdf)


def plot_orientations(pdf, poses, sparse_mapping_poses, ar_tag_poses):
    orientation_plotter = vector3d_plotter.Vector3dPlotter(
        "Time (s)",
        "Orientation (deg)",
        poses.pose_type + " vs. Sparse Mapping Orientation",
        True,
    )
    orientation_plotter.add_pose_orientation(
        sparse_mapping_poses,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    if ar_tag_poses.times:
        orientation_plotter.add_pose_orientation(
            ar_tag_poses,
            linestyle="None",
            marker="x",
            markeredgewidth=0.1,
            markersize=1.5,
        )
    orientation_plotter.add_pose_orientation(poses)
    orientation_plotter.plot(pdf)


def plot_covariances(pdf, times, covariances, name):
    plt.figure()
    title = name + " Covariance"
    plt.plot(times, l2_map(covariances), "r", linewidth=0.5, label=title)
    plt.title(title)
    plt.xlabel("Time (s)")
    plt.ylabel(title)
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()


def plot_features(
    feature_counts,
    times,
    label,
    color,
    linestyle="None",
    marker="o",
    markeredgewidth=0.1,
    markersize=1.5,
):
    plt.plot(
        times,
        feature_counts,
        color,
        linestyle=linestyle,
        marker=marker,
        markeredgewidth=markeredgewidth,
        markersize=markersize,
        label=label,
    )


def add_feature_count_plots(pdf, graph_localization_states):
    plt.figure()
    plot_features(
        graph_localization_states.num_detected_ml_features,
        graph_localization_states.times,
        "Det. VL",
        "b",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plot_features(
        graph_localization_states.num_ml_projection_factors,
        graph_localization_states.times,
        "VL Proj Factors",
        "r",
        marker="x",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plt.xlabel("Time (s)")
    plt.ylabel("ML Feature Counts")
    plt.title("ML Feature Counts")
    plt.legend(prop={"size": 6})
    plt.ylim(ymin=-1)
    pdf.savefig()
    plt.close()

    plt.figure()
    plot_features(
        graph_localization_states.num_detected_of_features,
        graph_localization_states.times,
        "Det. OF",
        "b",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plot_features(
        graph_localization_states.num_of_factors,
        graph_localization_states.times,
        "OF Factors",
        "r",
        marker="x",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Optical Flow Feature Counts")
    plt.title("Optical Flow Feature Counts")
    plt.legend(prop={"size": 6})
    plt.ylim(ymin=-1)
    pdf.savefig()
    plt.close()


def add_other_vector3d_plots(
    pdf, imu_augmented_graph_localization_states, sparse_mapping_poses, ar_tag_poses
):
    colors = ["r", "b", "g"]

    # Acceleration
    plt.figure()
    plot_helpers.plot_vector3ds(
        imu_augmented_graph_localization_states.accelerations,
        imu_augmented_graph_localization_states.times,
        "Acc.",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Acceleration (m/s^2)")
    plt.title("Acceleration")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Biases
    # Plot Accelerometer Biases on different pages since they can start with quite different
    # values, plotting on the same page will lead to a large y axis scale and hide subtle changes.
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.accelerometer_biases.xs,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Accelerometer Biases (X)")
    plt.title("Accelerometer Biases (X)")
    pdf.savefig()
    plt.close()

    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.accelerometer_biases.ys,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Accelerometer Biases (Y)")
    plt.title("Accelerometer Biases (Y)")
    pdf.savefig()
    plt.close()

    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.accelerometer_biases.zs,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Accelerometer Biases (Z)")
    plt.title("Accelerometer Biases (Z)")
    pdf.savefig()
    plt.close()

    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.gyro_biases.xs,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Gyro Biases (X)")
    plt.title("Gyro Biases (X)")
    pdf.savefig()
    plt.close()

    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.gyro_biases.ys,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Gyro Biases (Y)")
    plt.title("Gyro Biases (Y)")
    pdf.savefig()
    plt.close()

    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        imu_augmented_graph_localization_states.gyro_biases.zs,
        "r",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Gyro Biases (Z)")
    plt.title("Gyro Biases (Z)")
    pdf.savefig()
    plt.close()

    # Angular Velocity
    plt.figure()
    plot_helpers.plot_vector3ds(
        imu_augmented_graph_localization_states.angular_velocities,
        imu_augmented_graph_localization_states.times,
        "Ang. Vel.",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Angular Velocities")
    plt.title("Angular Velocities")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Velocity
    plt.figure()
    plot_helpers.plot_vector3ds(
        imu_augmented_graph_localization_states.velocities,
        imu_augmented_graph_localization_states.times,
        "Vel.",
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Velocities")
    plt.title("IMU Augmented Graph Velocities")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Integrated Velocities
    plt.figure()
    plot_helpers.plot_positions(
        sparse_mapping_poses,
        colors,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    if ar_tag_poses.times:
        plot_helpers.plot_positions(
            ar_tag_poses,
            colors,
            linestyle="None",
            marker="x",
            markeredgewidth=0.1,
            markersize=1.5,
        )
    integrated_imu_augmented_graph_localization_states = utilities.integrate_velocities(
        imu_augmented_graph_localization_states
    )
    plot_helpers.plot_positions(
        integrated_imu_augmented_graph_localization_states, colors, linewidth=0.5
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Position (m)")
    plt.title("Integrated IMU Augmented Graph Velocities vs. Sparse Mapping Position")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Position covariance
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        l2_map(imu_augmented_graph_localization_states.position_covariances),
        "r",
        linewidth=0.5,
        label="Position Covariance",
    )
    plt.title("Position Covariance")
    plt.xlabel("Time (s)")
    plt.ylabel("Position Covariance")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Orientation covariance
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        l2_map(imu_augmented_graph_localization_states.orientation_covariances),
        "r",
        linewidth=0.5,
        label="Orientation Covariance",
    )
    plt.title("Orientation Covariance (Quaternion)")
    plt.xlabel("Time (s)")
    plt.ylabel("Orientation Covariance")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Velocity covariance
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        l2_map(imu_augmented_graph_localization_states.velocity_covariances),
        "r",
        linewidth=0.5,
        label="Velocity Covariance",
    )
    plt.title("Velocity Covariance")
    plt.xlabel("Time (s)")
    plt.ylabel("Velocity Covariance")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Accel Bias covariance
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        l2_map(imu_augmented_graph_localization_states.accelerometer_bias_covariances),
        "r",
        linewidth=0.5,
        label="Accelerometer Bias Covariance",
    )
    plt.title("Accelerometer Bias Covariance")
    plt.xlabel("Time (s)")
    plt.ylabel("Accelerometer Bias Covariance")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # Gyro Bias covariance
    plt.figure()
    plt.plot(
        imu_augmented_graph_localization_states.times,
        l2_map(imu_augmented_graph_localization_states.gyro_bias_covariances),
        "r",
        linewidth=0.5,
        label="Gyro Bias Covariance",
    )
    plt.title("Gyro Bias Covariance")
    plt.xlabel("Time (s)")
    plt.ylabel("Gyro Bias Covariance")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()


def plot_loc_state_stats(
    pdf,
    localization_states,
    sparse_mapping_poses,
    output_csv_file,
    prefix="",
    atol=0,
    plot_integrated_velocities=True,
    rmse_rel_start_time=0,
    rmse_rel_end_time=-1,
):
    plot_loc_state_stats_abs(
        pdf,
        localization_states,
        sparse_mapping_poses,
        output_csv_file,
        prefix,
        atol,
        plot_integrated_velocities,
        rmse_rel_start_time,
        rmse_rel_end_time,
    )
    plot_loc_state_stats_rel(
        pdf,
        localization_states,
        sparse_mapping_poses,
        output_csv_file,
        prefix,
        atol,
        plot_integrated_velocities,
        rmse_rel_start_time,
        rmse_rel_end_time,
    )


def plot_loc_state_stats_abs(
    pdf,
    localization_states,
    sparse_mapping_poses,
    output_csv_file,
    prefix="",
    atol=0,
    plot_integrated_velocities=True,
    rmse_rel_start_time=0,
    rmse_rel_end_time=-1,
):
    rmse = rmse_utilities.rmse_timestamped_poses(
        localization_states,
        sparse_mapping_poses,
        True,
        atol,
        rmse_rel_start_time,
        rmse_rel_end_time,
    )
    integrated_rmse = []
    if plot_integrated_velocities:
        integrated_localization_states = utilities.integrate_velocities(
            localization_states
        )
        integrated_rmse = rmse_utilities.rmse_timestamped_poses(
            integrated_localization_states,
            sparse_mapping_poses,
            False,
            atol,
            rmse_rel_start_time,
            rmse_rel_end_time,
        )
    stats = (
        prefix
        + " pos rmse: "
        + str(rmse[0])
        + "\n"
        + "orientation rmse: "
        + str(rmse[1])
    )
    if plot_integrated_velocities:
        stats += "\n" + "integrated rmse: " + str(integrated_rmse[0])
    with open(output_csv_file, "a") as output_csv:
        csv_writer = csv.writer(output_csv, lineterminator="\n")
        csv_writer.writerow([prefix + "rmse", str(rmse[0])])
        csv_writer.writerow([prefix + "orientation_rmse", str(rmse[1])])
        if plot_integrated_velocities:
            csv_writer.writerow([prefix + "integrated_rmse", str(integrated_rmse[0])])
    plt.figure()
    plt.axis("off")
    plt.text(0.0, 0.5, stats)
    pdf.savefig()


def plot_loc_state_stats_rel(
    pdf,
    localization_states,
    sparse_mapping_poses,
    output_csv_file,
    prefix="",
    atol=0,
    plot_integrated_velocities=True,
    rmse_rel_start_time=0,
    rmse_rel_end_time=-1,
):
    rmse = rmse_utilities.rmse_timestamped_poses_relative(
        localization_states,
        sparse_mapping_poses,
        True,
        atol,
        rmse_rel_start_time,
        rmse_rel_end_time,
    )
    integrated_rmse = []
    if plot_integrated_velocities:
        integrated_localization_states = utilities.integrate_velocities(
            localization_states
        )
        integrated_rmse = rmse_utilities.rmse_timestamped_poses_relative(
            integrated_localization_states,
            sparse_mapping_poses,
            False,
            atol,
            rmse_rel_start_time,
            rmse_rel_end_time,
        )
    stats = (
        prefix
        + " rel pos rmse: "
        + str(rmse[0])
        + "\n"
        + "rel orientation rmse: "
        + str(rmse[1])
    )
    if plot_integrated_velocities:
        stats += "\n" + "rel integrated rmse: " + str(integrated_rmse[0])

    with open(output_csv_file, "a") as output_csv:
        csv_writer = csv.writer(output_csv, lineterminator="\n")
        csv_writer.writerow(["rel_" + prefix + "rmse", str(rmse[0])])
        csv_writer.writerow(["rel_" + prefix + "orientation_rmse", str(rmse[1])])
        if plot_integrated_velocities:
            csv_writer.writerow(
                ["rel_" + prefix + "integrated_rmse", str(integrated_rmse[0])]
            )
    plt.figure()
    plt.axis("off")
    plt.text(0.0, 0.5, stats)
    pdf.savefig()


def add_imu_bias_tester_poses(pdf, imu_bias_tester_poses, sparse_mapping_poses):
    colors = ["r", "b", "g"]
    plt.figure()
    plot_helpers.plot_positions(
        sparse_mapping_poses,
        colors,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plot_helpers.plot_positions(imu_bias_tester_poses, colors, linewidth=0.5)
    plt.xlabel("Time (s)")
    plt.ylabel("Position (m)")
    plt.title("Imu Bias Tester vs. Sparse Mapping Position")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()

    # orientations
    plt.figure()
    plot_helpers.plot_orientations(
        sparse_mapping_poses,
        colors,
        linestyle="None",
        marker="o",
        markeredgewidth=0.1,
        markersize=1.5,
    )
    plot_helpers.plot_orientations(imu_bias_tester_poses, colors, linewidth=0.5)
    plt.xlabel("Time (s)")
    plt.ylabel("Orienation (deg)")
    plt.title("Imu Bias Tester vs. Sparse Mapping Orientation")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()


def add_imu_bias_tester_velocities(pdf, imu_bias_tester_velocities):
    plt.figure()
    plot_helpers.plot_vector3ds(
        imu_bias_tester_velocities.velocities, imu_bias_tester_velocities.times, "Vel."
    )
    plt.xlabel("Time (s)")
    plt.ylabel("Velocities")
    plt.title("IMU Bias Tester Velocities")
    plt.legend(prop={"size": 6})
    pdf.savefig()
    plt.close()


def add_other_loc_plots(
    pdf,
    graph_localization_states,
    imu_augmented_graph_localization_states,
    sparse_mapping_poses,
    ar_tag_poses,
):
    add_feature_count_plots(pdf, graph_localization_states)
    add_other_vector3d_plots(
        pdf, imu_augmented_graph_localization_states, sparse_mapping_poses, ar_tag_poses
    )


def load_pose_msgs(vec_of_poses, bag, bag_start_time):
    topics = [poses.topic for poses in vec_of_poses]
    for topic, msg, t in bag.read_messages(topics):
        for poses in vec_of_poses:
            if poses.topic == topic:
                poses.add_msg(msg, msg.header.stamp, bag_start_time)
                break


def load_odometry_msgs(vec_of_poses, bag, bag_start_time):
    topics = [poses.topic for poses in vec_of_poses]
    for topic, msg, t in bag.read_messages(topics):
        for poses in vec_of_poses:
            if poses.topic == topic:
                poses.add_msg_with_covariance(
                    msg.odometry.body_F_source_T_target,
                    msg.header.stamp,
                    bag_start_time,
                )
                break


def load_pose_with_cov_msgs(vec_of_poses, bag, bag_start_time):
    topics = [poses.topic for poses in vec_of_poses]
    for topic, msg, t in bag.read_messages(topics):
        for poses in vec_of_poses:
            if poses.topic == topic:
                poses.add_msg_with_covariance(
                    msg.pose, msg.header.stamp, bag_start_time
                )
                break


def load_loc_state_msgs(vec_of_loc_states, bag, bag_start_time):
    topics = [loc_states.topic for loc_states in vec_of_loc_states]
    for topic, msg, t in bag.read_messages(topics):
        for loc_states in vec_of_loc_states:
            if loc_states.topic == topic:
                loc_states.add_loc_state(msg, bag_start_time)
                break


def load_velocity_msgs(velocities, bag, bag_start_time):
    topics = [velocities.topic]
    for topic, msg, t in bag.read_messages(topics):
        velocities.add_msg(msg, msg.header.stamp, bag_start_time)


def has_topic(bag, topic):
    topics = bag.get_type_and_topic_info().topics
    return topic in topics


# Groundtruth bag must have the same start time as other bagfile, otherwise RMSE calculations will be flawed
def create_plots(
    bagfile,
    output_pdf_file,
    output_csv_file="results.csv",
    groundtruth_bagfile=None,
    rmse_rel_start_time=0,
    rmse_rel_end_time=-1,
):
    bag = rosbag.Bag(bagfile)
    groundtruth_bag = rosbag.Bag(groundtruth_bagfile) if groundtruth_bagfile else bag
    bag_start_time = bag.get_start_time()

    has_imu_augmented_graph_localization_state = has_topic(bag, "/gnc/ekf")
    has_imu_bias_tester_poses = has_topic(bag, "/imu_bias_tester/pose")
    sparse_mapping_poses = poses.Poses("Sparse Mapping", "/sparse_mapping/pose")
    ar_tag_poses = poses.Poses("AR Tag", "/ar_tag/pose")
    imu_bias_tester_poses = poses.Poses("Imu Bias Tester", "/imu_bias_tester/pose")
    vec_of_poses = [ar_tag_poses, imu_bias_tester_poses]
    load_pose_msgs(vec_of_poses, bag, bag_start_time)
    has_depth_odom = has_topic(bag, "/loc/depth/odom")
    depth_odom_relative_poses = poses.Poses("Depth Odom", "/loc/depth/odom")
    load_odometry_msgs([depth_odom_relative_poses], bag, bag_start_time)
    groundtruth_vec_of_poses = [sparse_mapping_poses]
    load_pose_msgs(groundtruth_vec_of_poses, groundtruth_bag, bag_start_time)

    graph_localization_states = loc_states.LocStates(
        "Graph Localization", "/graph_loc/state"
    )
    imu_augmented_graph_localization_states = loc_states.LocStates(
        "Imu Augmented Graph Localization", "/gnc/ekf"
    )
    vec_of_loc_states = [
        graph_localization_states,
        imu_augmented_graph_localization_states,
    ]
    load_loc_state_msgs(vec_of_loc_states, bag, bag_start_time)

    imu_bias_tester_velocities = velocities.Velocities(
        "Imu Bias Tester", "/imu_bias_tester/velocity"
    )
    load_velocity_msgs(imu_bias_tester_velocities, bag, bag_start_time)

    bag.close()

    with PdfPages(output_pdf_file) as pdf:
        add_graph_plots(
            pdf,
            sparse_mapping_poses,
            ar_tag_poses,
            graph_localization_states,
            imu_augmented_graph_localization_states,
        )
        if has_imu_bias_tester_poses:
            add_imu_bias_tester_poses(pdf, imu_bias_tester_poses, sparse_mapping_poses)
            add_imu_bias_tester_velocities(pdf, imu_bias_tester_velocities)
        if has_imu_augmented_graph_localization_state:
            add_other_loc_plots(
                pdf,
                graph_localization_states,
                imu_augmented_graph_localization_states,
                sparse_mapping_poses,
                ar_tag_poses,
            )
        else:
            add_other_loc_plots(
                pdf, graph_localization_states, graph_localization_states
            )
        if has_depth_odom:
            depth_odom_poses = utilities.make_absolute_poses_from_relative_poses(
                sparse_mapping_poses, depth_odom_relative_poses, "Depth Odometry"
            )
            plot_poses(pdf, depth_odom_poses, sparse_mapping_poses, ar_tag_poses)
            # Note that for absolute time difference tolerance depth images and groudtruth use different sensor data
            # and therefore have less similar timestamps. This timestamp difference reduces the accuracy of depth odometry
            # groundtruth comparison.
            plot_loc_state_stats(
                pdf,
                depth_odom_poses,
                sparse_mapping_poses,
                output_csv_file,
                "depth_odometry_",
                0.01,
                False,
                rmse_rel_start_time=rmse_rel_start_time,
                rmse_rel_end_time=rmse_rel_end_time,
            )
            plot_covariances(
                pdf,
                depth_odom_relative_poses.times,
                depth_odom_relative_poses.covariances.position,
                "Depth Odometry Position",
            )
            plot_covariances(
                pdf,
                depth_odom_relative_poses.times,
                depth_odom_relative_poses.covariances.orientation,
                "Depth Odometry Orientation",
            )
        plot_loc_state_stats(
            pdf,
            graph_localization_states,
            sparse_mapping_poses,
            output_csv_file,
            rmse_rel_start_time=rmse_rel_start_time,
            rmse_rel_end_time=rmse_rel_end_time,
        )
        plot_loc_state_stats(
            pdf,
            imu_augmented_graph_localization_states,
            sparse_mapping_poses,
            output_csv_file,
            "imu_augmented_",
            0.01,
            rmse_rel_start_time=rmse_rel_start_time,
            rmse_rel_end_time=rmse_rel_end_time,
        )
        if has_imu_bias_tester_poses:
            plot_loc_state_stats(
                pdf,
                imu_bias_tester_poses,
                sparse_mapping_poses,
                output_csv_file,
                "imu_bias_tester_",
                0.01,
                False,
                rmse_rel_start_time=rmse_rel_start_time,
                rmse_rel_end_time=rmse_rel_end_time,
            )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("bagfile", help="Input bagfile.")
    parser.add_argument("--output-file", default="output.pdf", help="Output pdf file.")
    parser.add_argument(
        "--output-csv-file",
        default="results.csv",
        help="Output csv file containing localization stats.",
    )
    parser.add_argument(
        "-g",
        "--groundtruth-bagfile",
        default=None,
        help="Optional bagfile containing groundtruth poses to use as a comparison for poses in the input bagfile. If none provided, sparse mapping poses are used as groundtruth from the input bagfile if available.",
    )
    parser.add_argument(
        "--rmse-rel-start-time",
        type=float,
        default=0,
        help="Optional start time for plots.",
    )
    parser.add_argument(
        "--rmse-rel-end-time",
        type=float,
        default=-1,
        help="Optional end time for plots.",
    )
    args = parser.parse_args()
    if not os.path.isfile(args.bagfile):
        print(("Bag file " + args.bagfile + " does not exist."))
        sys.exit()
    if args.groundtruth_bagfile and not os.path.isfile(args.groundtruth_bagfile):
        print(("Groundtruth Bag file " + args.groundtruth_bagfile + " does not exist."))
        sys.exit()
    create_plots(
        args.bagfile,
        args.output_file,
        args.output_csv_file,
        args.groundtruth_bagfile,
        args.rmse_rel_start_time,
        args.rmse_rel_end_time,
    )
