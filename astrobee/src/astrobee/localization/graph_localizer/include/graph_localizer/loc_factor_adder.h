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

#ifndef GRAPH_LOCALIZER_LOC_FACTOR_ADDER_H_
#define GRAPH_LOCALIZER_LOC_FACTOR_ADDER_H_

#include <graph_localizer/loc_factor_adder_params.h>
#include <graph_optimizer/factor_adder.h>
#include <localization_common/averager.h>
#include <localization_measurements/matched_projections_measurement.h>

#include <vector>

namespace graph_localizer {
class LocFactorAdder : public graph_optimizer::FactorAdder<localization_measurements::MatchedProjectionsMeasurement,
                                                           LocFactorAdderParams> {
  using Base =
    graph_optimizer::FactorAdder<localization_measurements::MatchedProjectionsMeasurement, LocFactorAdderParams>;

 public:
  LocFactorAdder(const LocFactorAdderParams& params,
                 const graph_optimizer::GraphActionCompleterType graph_action_completer_type);

  std::vector<graph_optimizer::FactorsToAdd> AddFactors(
    const localization_measurements::MatchedProjectionsMeasurement& matched_projections_measurement) final;

 private:
  graph_optimizer::GraphActionCompleterType type() const;

  graph_optimizer::GraphActionCompleterType graph_action_completer_type_;
  localization_common::Averager num_landmarks_averager_ = localization_common::Averager("Num Landmarks");
};
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_LOC_FACTOR_ADDER_H_
