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
#include <localization_common/test_utilities.h>
#include <localization_common/utilities.h>
#include <vision_common/feature_image.h>
#include <vision_common/lk_optical_flow_feature_detector_and_matcher.h>
#include <vision_common/lk_optical_flow_feature_detector_and_matcher_params.h>
#include <vision_common/test_utilities.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <gtest/gtest.h>

namespace lc = localization_common;
namespace vc = vision_common;

TEST(FeatureDetectorAndMatcherTester, LKOpticalFlow) {
  auto params = vc::DefaultLKOpticalFlowFeatureDetectorAndMatcherParams();
  params.good_features_to_track.max_corners = 10000000;
  vc::LKOpticalFlowFeatureDetectorAndMatcher lk_detector_and_matcher(params);
  const int row_spacing = 33;
  const int col_spacing = 33;
  int num_markers_added;
  const cv::Point2i offset(5, 5);
  const auto image_a = vc::MarkerImage(row_spacing, col_spacing, num_markers_added);
  const auto image_b = vc::MarkerImage(row_spacing, col_spacing, num_markers_added, offset);
  vc::FeatureImage feature_image_a(image_a, *(lk_detector_and_matcher.detector()));
  vc::FeatureImage feature_image_b(image_b, *(lk_detector_and_matcher.detector()));
  const auto matches = lk_detector_and_matcher.Match(feature_image_a, feature_image_b);
  EXPECT_EQ(matches.size(), num_markers_added);
  for (const auto& match : matches) {
    const Eigen::Vector2d match_offset = match.target_point - match.source_point;
    EXPECT_NEAR(match_offset.x(), offset.x, 1e-1);
    EXPECT_NEAR(match_offset.y(), offset.y, 1e-1);
  }
}

// Run all the tests that were declared with TEST()
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
