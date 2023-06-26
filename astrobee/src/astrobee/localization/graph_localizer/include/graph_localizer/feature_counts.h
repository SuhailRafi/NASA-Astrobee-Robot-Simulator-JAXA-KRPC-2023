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

#ifndef GRAPH_LOCALIZER_FEATURE_COUNTS_H_
#define GRAPH_LOCALIZER_FEATURE_COUNTS_H_

namespace graph_localizer {

struct FeatureCounts {
  void Reset() {
    vl = 0;
    of = 0;
    ar = 0;
    depth = 0;
  }

  int vl = 0;
  int of = 0;
  int ar = 0;
  int depth = 0;
};
}  // namespace graph_localizer

#endif  // GRAPH_LOCALIZER_FEATURE_COUNTS_H_
