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

#ifndef GRAPH_OPTIMIZER_FACTOR_TO_ADD_H_
#define GRAPH_OPTIMIZER_FACTOR_TO_ADD_H_

#include <graph_optimizer/graph_action_completer_type.h>
#include <graph_optimizer/key_info.h>
#include <localization_common/time.h>

#include <gtsam/nonlinear/NonlinearFactor.h>

#include <vector>

namespace graph_optimizer {
struct FactorToAdd {
  FactorToAdd(const KeyInfos& key_infos, boost::shared_ptr<gtsam::NonlinearFactor> factor)
      : factor(factor), key_infos(key_infos) {}

  boost::shared_ptr<gtsam::NonlinearFactor> factor;
  KeyInfos key_infos;
};

class FactorsToAdd {
 public:
  FactorsToAdd(const localization_common::Time timestamp, const std::vector<FactorToAdd>& factors_to_add,
               const GraphActionCompleterType graph_action_completer_type = GraphActionCompleterType::None)
      : timestamp_(timestamp),
        factors_to_add_(factors_to_add),
        graph_action_completer_type_(graph_action_completer_type) {}

  explicit FactorsToAdd(const GraphActionCompleterType graph_action_completer_type = GraphActionCompleterType::None)
      : graph_action_completer_type_(graph_action_completer_type) {}

  void reserve(const int size) { factors_to_add_.reserve(size); }
  size_t size() const { return factors_to_add_.size(); }
  bool empty() const { return factors_to_add_.empty(); }
  void push_back(FactorToAdd&& factor_to_add) { factors_to_add_.emplace_back(std::move(factor_to_add)); }  // NOLINT
  void push_back(const FactorToAdd& factor_to_add) { factors_to_add_.push_back(factor_to_add); }
  void SetTimestamp(const localization_common::Time timestamp) { timestamp_ = timestamp; }

  localization_common::Time timestamp() const { return timestamp_; }
  const std::vector<FactorToAdd>& Get() const { return factors_to_add_; }
  std::vector<FactorToAdd>& Get() { return factors_to_add_; }
  GraphActionCompleterType graph_action_completer_type() const { return graph_action_completer_type_; }

 private:
  // Timestamp used to sort factors when adding to graph.
  localization_common::Time timestamp_;
  std::vector<FactorToAdd> factors_to_add_;
  GraphActionCompleterType graph_action_completer_type_;
};
}  // namespace graph_optimizer

#endif  // GRAPH_OPTIMIZER_FACTOR_TO_ADD_H_
