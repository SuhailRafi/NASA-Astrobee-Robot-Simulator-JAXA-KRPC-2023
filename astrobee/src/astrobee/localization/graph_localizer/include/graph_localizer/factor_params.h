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
#ifndef GRAPH_LOCALIZER_FACTOR_PARAMS_H_
#define GRAPH_LOCALIZER_FACTOR_PARAMS_H_

#include <graph_localizer/depth_odometry_factor_adder_params.h>
#include <graph_localizer/handrail_factor_adder_params.h>
#include <graph_localizer/loc_factor_adder_params.h>
#include <graph_localizer/rotation_factor_adder_params.h>
#include <graph_localizer/projection_factor_adder_params.h>
#include <graph_localizer/smart_projection_factor_adder_params.h>
#include <graph_localizer/standstill_factor_adder_params.h>

namespace graph_localizer {
struct FactorParams {
  DepthOdometryFactorAdderParams depth_odometry_adder;
  HandrailFactorAdderParams handrail_adder;
  RotationFactorAdderParams rotation_adder;
  SmartProjectionFactorAdderParams smart_projection_adder;
  StandstillFactorAdderParams standstill_adder;
  ProjectionFactorAdderParams projection_adder;
  LocFactorAdderParams loc_adder;
  LocFactorAdderParams ar_tag_loc_adder;
};
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_FACTOR_PARAMS_H_
