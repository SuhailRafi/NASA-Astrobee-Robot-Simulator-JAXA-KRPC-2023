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
#ifndef VISION_COMMON_BRISK_FEATURE_DETECTOR_AND_MATCHER_PARAMS_H_
#define VISION_COMMON_BRISK_FEATURE_DETECTOR_AND_MATCHER_PARAMS_H_

namespace vision_common {
struct BriskFeatureDetectorAndMatcherParams {
  // Detection
  int brisk_threshold;
  int brisk_octaves;
  float brisk_float_pattern_scale;
  // Matching
  int max_match_hamming_distance;
  int flann_table_number;
  int flann_key_size;
  int flann_multi_probe_level;
};
}  // namespace vision_common

#endif  // VISION_COMMON_BRISK_FEATURE_DETECTOR_AND_MATCHER_PARAMS_H_
