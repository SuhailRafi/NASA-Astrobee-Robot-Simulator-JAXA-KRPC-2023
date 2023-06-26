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

// Header file must go in src directory for Qt/Rviz plugin
#ifndef LOCALIZATION_RVIZ_PLUGINS_DEPTH_ODOMETRY_DISPLAY_H_  // NOLINT
#define LOCALIZATION_RVIZ_PLUGINS_DEPTH_ODOMETRY_DISPLAY_H_  // NOLINT

// Required for Qt
#ifndef Q_MOC_RUN
#include <QObject>
#include <camera/camera_params.h>
#include <ff_msgs/DepthOdometry.h>
#include <localization_common/measurement_buffer.h>
#include <localization_common/time.h>
#include <localization_measurements/depth_odometry_measurement.h>
#include <localization_measurements/measurement_conversions.h>
#include <image_transport/image_transport.h>
#include <opencv2/core.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <ros/publisher.h>
#include <ros/subscriber.h>
#include <sensor_msgs/PointCloud2.h>
#include <rviz/message_filter_display.h>
#include "slider_property.h"  // NOLINT
#endif

// Forward declarations for ogre and rviz
namespace Ogre {
class SceneNode;
}

namespace localization_rviz_plugins {
class DepthOdometryDisplay : public rviz::MessageFilterDisplay<ff_msgs::DepthOdometry> {
  Q_OBJECT    // NOLINT
    public :  // NOLINT
              DepthOdometryDisplay();
  ~DepthOdometryDisplay() = default;

  // private:
 protected:
  void onInitialize() final;
  void reset() final;

 private Q_SLOTS:  // NOLINT
  void createSingleCorrespondenceDisplays();

 private:
  void processMessage(const ff_msgs::DepthOdometry::ConstPtr& depth_odometry_msg);
  void createProjectionAndOpticalFlowImage(const localization_measurements::DepthOdometryMeasurement& depth_odometry);
  cv::Point2f projectPoint(const Eigen::Vector3d& point_3d);
  void imageCallback(const sensor_msgs::ImageConstPtr& image_msg);
  void pointCloudCallback(const sensor_msgs::PointCloud2ConstPtr& point_cloud_msg);
  void publishCorrespondencePoints(const Eigen::Vector3d& source_3d_point, const Eigen::Vector3d& target_3d_point,
                                   const localization_common::Time source_time,
                                   const localization_common::Time target_time);
  void clearDisplay();

  std::unique_ptr<camera::CameraParameters> camera_params_;
  std::unique_ptr<rviz::SliderProperty> correspondence_index_slider_;
  cv::Mat intrinsics_;
  cv::Mat distortion_params_;
  image_transport::Subscriber image_sub_;
  ros::Subscriber point_cloud_sub_;
  ros::Publisher source_correspondence_point_pub_, target_correspondence_point_pub_;
  ros::Publisher source_point_cloud_pub_, target_point_cloud_pub_;
  image_transport::Publisher single_correspondence_image_pub_, projection_and_optical_flow_image_pub_;
  ros::NodeHandle nh_;
  localization_common::MeasurementBuffer<sensor_msgs::ImageConstPtr> img_buffer_;
  localization_common::MeasurementBuffer<pcl::PointCloud<pcl::PointXYZ>::Ptr> point_cloud_buffer_;
  boost::optional<localization_measurements::DepthOdometryMeasurement> latest_depth_odometry_measurement_;
};
}  // namespace localization_rviz_plugins
#endif  // LOCALIZATION_RVIZ_PLUGINS_DEPTH_ODOMETRY_DISPLAY_H_ NOLINT
