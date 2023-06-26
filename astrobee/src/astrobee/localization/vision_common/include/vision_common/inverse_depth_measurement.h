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

#ifndef VISION_COMMON_INVERSE_DEPTH_MEASUREMENT_H_
#define VISION_COMMON_INVERSE_DEPTH_MEASUREMENT_H_

#include <vision_common/utilities.h>

#include <gtsam/base/Lie.h>
#include <gtsam/base/Manifold.h>
#include <gtsam/base/Matrix.h>
#include <gtsam/geometry/Pose3.h>

#include <string>

namespace vision_common {

/**
 * Optimizable inverse depth parameterization for a landmark point using a (u,v)
 * image space measurement and the associated camera pose for the measurement
 * along with the camera intrinsics matrix to be able to backproject the inverse depth
 * point to a 3d point.
 */

class InverseDepthMeasurement {
 public:
  // Required for gtsam optimization
  enum { dimension = 1 };

  InverseDepthMeasurement(const double inverse_depth, const Eigen::Vector2d& image_coordinates,
                          const Eigen::Matrix3d& intrinsics, const gtsam::Pose3& body_T_sensor)
      : inverse_depth_(inverse_depth),
        image_coordinates_(image_coordinates),
        intrinsics_(intrinsics),
        body_T_sensor_(body_T_sensor) {}

  // Returns sensor_t_point
  Eigen::Vector3d Backproject(boost::optional<gtsam::Matrix&> d_backprojected_point_d_depth = boost::none) const {
    return vision_common::Backproject(image_coordinates_, intrinsics_, depth(), d_backprojected_point_d_depth);
  }

  // Computes measurement projection in sensor frame of the provided target pose.
  boost::optional<Eigen::Vector2d> Project(
    const gtsam::Pose3& world_T_source_body, const gtsam::Pose3& world_T_target_body,
    boost::optional<gtsam::Matrix&> d_projected_point_d_world_T_source_body = boost::none,
    boost::optional<gtsam::Matrix&> d_projected_point_d_world_T_target_body = boost::none,
    boost::optional<gtsam::Matrix&> d_projected_point_d_inverse_depth = boost::none) const {
    if (d_projected_point_d_world_T_source_body || d_projected_point_d_world_T_target_body ||
        d_projected_point_d_inverse_depth) {
      // Jacobian Calculations:
      // projected_point = (project(target_sensor_T_source_sensor*backproject(inverse_depth)))
      // call backproject(inverse_depth) = source_sensor_t_point
      // call target_sensor_T_source_sensor* source_sensor_t_point = target_sensor_t_point
      // Pose Jacobians:
      // d_projected_point_d_world_T_source_body = d_projected_point_d_target_sensor_t_point *
      // d_target_sensor_t_point_d_world_T_source_sensor * d_world_T_source_sensor_d_world_T_source_body
      // d_projected_point_d_world_T_target_body = d_projected_point_d_target_sensor_t_point *
      // d_target_sensor_t_point_d_world_T_target_sensor * d_world_T_target_sensor_d_world_T_target_body
      // where:
      // d_target_sensor_t_point_d_world_T_source_sensor = d_target_sensor_t_point_d_target_sensor_T_source_sensor *
      // d_target_sensor_T_source_sensor_d_world_T_source_sensor d_target_sensor_t_point_d_world_T_target_sensor =
      // d_target_sensor_t_point_d_target_sensor_T_source_sensor *
      // d_target_sensor_T_source_sensor_d_world_T_target_sensor d_target_sensor_T_source_sensor_d_world_T_target_sensor
      // = d_target_sensor_T_source_sensor_d_target_sensor_T_world * d_target_sensor_T_world_d_world_T_target_sensor
      // Inverse Depth Jacobian:
      // d_projected_point_d_inverse_depth = d_projected_point_d_target_sensor_t_point *
      // d_target_sensor_t_point_d_depth * d_depth_d_inverse_depth
      // where:
      // d_target_sensor_t_point_d_depth =
      // d_target_sensor_t_point_d_source_sensor_t_point * d_source_sensor_t_point_d_depth
      // d_depth_d_inverse_depth = -1/(inverse_depth^2)

      // Intermediate Jacobians
      gtsam::Matrix d_source_sensor_t_point_d_depth;
      gtsam::Matrix d_target_sensor_T_world_d_world_T_target_sensor;
      gtsam::Matrix d_target_sensor_T_source_sensor_d_world_T_source_sensor;
      gtsam::Matrix d_target_sensor_T_source_sensor_d_target_sensor_T_world;
      gtsam::Matrix d_target_sensor_t_point_d_target_sensor_T_source_sensor;
      gtsam::Matrix d_target_sensor_t_point_d_source_sensor_t_point;
      gtsam::Matrix d_projected_point_d_target_sensor_t_point;
      gtsam::Matrix d_world_T_source_sensor_d_world_T_source_body;
      gtsam::Matrix d_world_T_target_sensor_d_world_T_target_body;
      const auto projeced_point =
        ProjectHelper(world_T_source_body, world_T_target_body, d_world_T_source_sensor_d_world_T_source_body,
                      d_world_T_target_sensor_d_world_T_target_body, d_target_sensor_T_world_d_world_T_target_sensor,
                      d_source_sensor_t_point_d_depth, d_target_sensor_T_source_sensor_d_target_sensor_T_world,
                      d_target_sensor_T_source_sensor_d_world_T_source_sensor,
                      d_target_sensor_t_point_d_target_sensor_T_source_sensor,
                      d_target_sensor_t_point_d_source_sensor_t_point, d_projected_point_d_target_sensor_t_point);
      // Final pose Jacobians
      const gtsam::Matrix d_target_sensor_T_source_sensor_d_world_T_target_sensor =
        d_target_sensor_T_source_sensor_d_target_sensor_T_world * d_target_sensor_T_world_d_world_T_target_sensor;
      const gtsam::Matrix d_target_sensor_t_point_d_world_T_source_sensor =
        d_target_sensor_t_point_d_target_sensor_T_source_sensor *
        d_target_sensor_T_source_sensor_d_world_T_source_sensor;
      const gtsam::Matrix d_target_sensor_t_point_d_world_T_target_sensor =
        d_target_sensor_t_point_d_target_sensor_T_source_sensor *
        d_target_sensor_T_source_sensor_d_world_T_target_sensor;
      if (d_projected_point_d_world_T_source_body)
        *d_projected_point_d_world_T_source_body = d_projected_point_d_target_sensor_t_point *
                                                   d_target_sensor_t_point_d_world_T_source_sensor *
                                                   d_world_T_source_sensor_d_world_T_source_body;
      if (d_projected_point_d_world_T_target_body)
        *d_projected_point_d_world_T_target_body = d_projected_point_d_target_sensor_t_point *
                                                   d_target_sensor_t_point_d_world_T_target_sensor *
                                                   d_world_T_target_sensor_d_world_T_target_body;
      // Final inverse depth Jacobian
      const double d_depth_d_inverse_depth = -1.0 / (inverse_depth_ * inverse_depth_);
      const gtsam::Matrix d_target_sensor_t_point_d_depth =
        d_target_sensor_t_point_d_source_sensor_t_point * d_source_sensor_t_point_d_depth;
      if (d_projected_point_d_inverse_depth)
        *d_projected_point_d_inverse_depth =
          d_projected_point_d_target_sensor_t_point * d_target_sensor_t_point_d_depth * d_depth_d_inverse_depth;
      return projeced_point;
    }

    // Jacobians not required
    return ProjectHelper(world_T_source_body, world_T_target_body);
  }

  double depth() const { return 1.0 / inverse_depth_; }

  double inverse_depth() const { return inverse_depth_; }

  // Required operations for using as state parameter with gtsam
  inline size_t dim() const { return dimension; }

  static size_t Dim() { return dimension; }

  // Boxplus
  inline InverseDepthMeasurement retract(const gtsam::Vector& d) const {
    return InverseDepthMeasurement(inverse_depth_ + d(0), image_coordinates_, intrinsics_, body_T_sensor_);
  }

  // Boxminus
  gtsam::Vector1 localCoordinates(const InverseDepthMeasurement& T2) const {
    return gtsam::Vector1(T2.inverse_depth() - inverse_depth());
  }

  void print(const std::string& s = std::string()) const {
    std::cout << "inverse depth: " << inverse_depth_ << std::endl;
  }

  bool equals(const InverseDepthMeasurement& s, double tol = 1e-9) const {
    return intrinsics_.isApprox(s.intrinsics_, tol) && body_T_sensor_.equals(s.body_T_sensor_, 1e-9) &&
           std::abs(inverse_depth_ - s.inverse_depth_) < 1e-9 &&
           image_coordinates_.isApprox(s.image_coordinates_, 1e-9);
  }

 private:
  // Intermediate call to optionally fill required jacobians.  Allows for code reuse whether the jacobians are need or
  // not.
  boost::optional<Eigen::Vector2d> ProjectHelper(
    const gtsam::Pose3& world_T_source_body, const gtsam::Pose3& world_T_target_body,
    boost::optional<gtsam::Matrix&> d_world_T_source_sensor_d_world_T_source_body = boost::none,
    boost::optional<gtsam::Matrix&> d_world_T_target_sensor_d_world_T_target_body = boost::none,
    boost::optional<gtsam::Matrix&> d_target_sensor_T_world_d_world_T_target_sensor = boost::none,
    boost::optional<gtsam::Matrix&> d_source_sensor_t_point_d_depth = boost::none,
    boost::optional<gtsam::Matrix&> d_target_sensor_T_source_sensor_d_target_sensor_T_world = boost::none,
    boost::optional<gtsam::Matrix&> d_target_sensor_T_source_sensor_d_world_T_source_sensor = boost::none,
    boost::optional<gtsam::Matrix&> d_target_sensor_t_point_d_target_sensor_T_source_sensor = boost::none,
    boost::optional<gtsam::Matrix&> d_target_sensor_t_point_d_source_sensor_t_point = boost::none,
    boost::optional<gtsam::Matrix&> d_projected_point_d_target_sensor_t_point = boost::none) const {
    const gtsam::Pose3 world_T_source_sensor =
      world_T_source_body.compose(body_T_sensor_, d_world_T_source_sensor_d_world_T_source_body);
    const gtsam::Pose3 world_T_target_sensor =
      world_T_target_body.compose(body_T_sensor_, d_world_T_target_sensor_d_world_T_target_body);
    const Eigen::Vector3d source_sensor_t_point = Backproject(d_source_sensor_t_point_d_depth);
    const gtsam::Pose3 target_sensor_T_world =
      world_T_target_sensor.inverse(d_target_sensor_T_world_d_world_T_target_sensor);
    const gtsam::Pose3 target_sensor_T_source_sensor =
      target_sensor_T_world.compose(world_T_source_sensor, d_target_sensor_T_source_sensor_d_target_sensor_T_world,
                                    d_target_sensor_T_source_sensor_d_world_T_source_sensor);
    const Eigen::Vector3d target_sensor_t_point = target_sensor_T_source_sensor.transformFrom(
      source_sensor_t_point, d_target_sensor_t_point_d_target_sensor_T_source_sensor,
      d_target_sensor_t_point_d_source_sensor_t_point);
    if (target_sensor_t_point.z() < 0) return boost::none;
    return vision_common::Project(target_sensor_t_point, intrinsics_, d_projected_point_d_target_sensor_t_point);
  }

  Eigen::Vector2d image_coordinates_;
  Eigen::Matrix3d intrinsics_;
  gtsam::Pose3 body_T_sensor_;
  double inverse_depth_;
};
}  // namespace vision_common

namespace gtsam {
template <>
struct traits<vision_common::InverseDepthMeasurement>
    : public internal::Manifold<vision_common::InverseDepthMeasurement> {};

template <>
struct traits<const vision_common::InverseDepthMeasurement>
    : public internal::Manifold<vision_common::InverseDepthMeasurement> {};
}  // namespace gtsam

#endif  // VISION_COMMON_INVERSE_DEPTH_MEASUREMENT_H_
