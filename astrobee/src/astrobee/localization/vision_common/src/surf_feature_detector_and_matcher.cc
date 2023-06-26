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

#include <localization_common/logger.h>
#include <vision_common/surf_feature_detector_and_matcher.h>
#include <vision_common/surf_feature_detector_and_matcher_params.h>

#include <opencv2/xfeatures2d.hpp>

namespace vision_common {
SurfFeatureDetectorAndMatcher::SurfFeatureDetectorAndMatcher(const SurfFeatureDetectorAndMatcherParams& params)
    : params_(params) {
  detector_ = cv::xfeatures2d::SURF::create(params_.surf_threshold);
}
FeatureMatches SurfFeatureDetectorAndMatcher::Match(const FeatureImage& source_image,
                                                    const FeatureImage& target_image) {
  std::vector<cv::DMatch> matches;
  flann_matcher_.match(source_image.descriptors(), target_image.descriptors(), matches);
  const auto filtered_end = std::remove_if(matches.begin(), matches.end(), [this](const cv::DMatch& match) {
    return match.distance > params_.max_match_distance;
  });
  matches.erase(filtered_end, matches.end());
  FeatureMatches feature_matches;
  for (const auto& match : matches) {
    const auto& source_point = source_image.keypoints()[match.queryIdx].pt;
    const auto& target_point = target_image.keypoints()[match.trainIdx].pt;
    feature_matches.emplace_back(Eigen::Vector2d(source_point.x, source_point.y),
                                 Eigen::Vector2d(target_point.x, target_point.y), match.distance);
  }
  return feature_matches;
}
}  // namespace vision_common
