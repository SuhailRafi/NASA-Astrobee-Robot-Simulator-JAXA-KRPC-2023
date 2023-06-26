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

#include <ff_common/init.h>
#include <ff_common/thread.h>
#include <ff_common/utils.h>
#include <config_reader/config_reader.h>
#include <camera/camera_params.h>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/filesystem.hpp>
#include <string>
#include <iostream>

/*
  Undistort camera images for a robot.
  Usage:

  export ASTROBEE_RESOURCE_DIR=$SOURCE_PATH/astrobee/resources
  export ASTROBEE_CONFIG_DIR=$SOURCE_PATH/astrobee/config
  export ASTROBEE_WORLD=granite
  export ASTROBEE_ROBOT=p4d
  undistort_image *[0-9].jpg

  Distorted images will be written to the same directory, unless a
  separate directory is specified.
*/

DEFINE_string(image_list, "", "A file having the list of images to undistort, one per line. "
              "If not specified, it is assumed they are passed in directly on the command line.");

DEFINE_string(output_directory, "", "Output directory. If not specified, undistorted images will "
              "saved in the same directory as the inputs.");

DEFINE_string(output_list, "", "Save the undistorted images with names given in this list, "
              "instead of using the output directory.");

DEFINE_string(undistorted_intrinsics, "", "Save to this file the undistorted camera intrinsics.");

DEFINE_double(scale, 1.0, "Undistort images at different resolution, with their width "
              "being a multiple of this scale compared to the camera model.");

DEFINE_string(undistorted_crop_win, "",
              "After undistorting, apply a crop window of these dimensions "
              "centered at the undistorted image center. The adjusted "
              "dimensions and optical center will be printed on screen. "
              "Specify as: 'crop_x crop_y'.");

DEFINE_bool(save_bgr, false,
            "Save the undistorted images as BGR instead of grayscale. (Some tools expect BGR.)");

DEFINE_bool(histogram_equalization, false,
            "If true, do histogram equalization.");

DEFINE_string(robot_camera, "nav_cam",
              "Which of bot's cameras to use. Tested with nav_cam and sci_cam.");

int main(int argc, char ** argv) {
  ff_common::InitFreeFlyerApplication(&argc, &argv);

  // The images can either be in a list or passed in
  std::vector<std::string> images;
  if (FLAGS_image_list != "") {
    std::ifstream ifs(FLAGS_image_list);
    std::string image;
    while (ifs >> image)
      images.push_back(image);
  } else {
    for (int i = 1; i < argc; i++)
      images.push_back(argv[i]);
  }

  if (images.empty())
      LOG(FATAL) << "Expecting at least one input image.";

  std::vector<std::string> undist_images;
  if (FLAGS_output_list != "") {
    std::ifstream ifs(FLAGS_output_list);
    std::string image;
    while (ifs >> image)
      undist_images.push_back(image);

    if (undist_images.size() != images.size())
      LOG(FATAL) << "There must be as many output undistorted "
                 << "images as input distorted images.\n";
  }

  // Read the camera
  config_reader::ConfigReader config;
  config.AddFile("cameras.config");
  if (!config.ReadFiles()) {
    LOG(FATAL) << "Failed to read config files.";
  }
  camera::CameraParameters cam_params(&config, FLAGS_robot_camera.c_str());

  // Useful for over-riding any config files when debugging
  // camera::CameraParameters cam_params(Eigen::Vector2i(776, 517),
  //                                    Eigen::Vector2d::Constant(610.502),
  //                                    Eigen::Vector2d(776/2.0, 517/2.0));

  if (!FLAGS_output_directory.empty()) {
    if (!boost::filesystem::create_directories(FLAGS_output_directory) &&
       !boost::filesystem::exists(FLAGS_output_directory)) {
      LOG(FATAL) << "Failed to create directory: " << FLAGS_output_directory;
    }
  }

  // Create the undistortion map
  cv::Mat floating_remap, fixed_map, interp_map;
  cam_params.GenerateRemapMaps(&floating_remap, FLAGS_scale);

  // We have to conform to the OpenCV API, which says:
  // undist_image(x, y) = dist_image(floating_remap(x, y)).

  // If floating_remap(x, y) is out of dist_image bounds, the above
  // should return a black pixel, yet a straightforward application of
  // this formula will result in a segfault.

  // The solution is to grow dist_image by padding it with black pixels
  // so that the above API succeeds.

  // This can have the following problem though. floating_remap(x, y)
  // can be huge for unreasonable distortion. So tame it. We only
  // want to know that if this falls outside the image bounds, a black
  // pixel is assigned to undist_image(x, y), so if it falls too
  // much outside the image just assign it to a closer pixel outside
  // the image.
  float max_extra = 100.0f;  // the furthest floating_remap(x, y) can deviate

  // The image dimensions
  Eigen::Vector2i dims(round(FLAGS_scale*cam_params.GetDistortedSize()[0]),
                       round(FLAGS_scale*cam_params.GetDistortedSize()[1]));
  int img_cols = dims[0], img_rows = dims[1];

  cv::Vec2f start = floating_remap.at<cv::Vec2f>(0, 0);
  double min_x = 0.0, max_x = 0.0, min_y = 0.0, max_y = 0.0;  // will change very soon
  for (int col = 0; col < floating_remap.cols; col++) {
    for (int row = 0; row < floating_remap.rows; row++) {
      cv::Vec2f pix = floating_remap.at<cv::Vec2f>(row, col);

      // Tame floating_remap
      pix[0] = std::max(pix[0], -max_extra);
      pix[0] = std::min(pix[0], img_cols + max_extra);
      pix[1] = std::max(pix[1], -max_extra);
      pix[1] = std::min(pix[1], img_rows + max_extra);
      floating_remap.at<cv::Vec2f>(row, col) = pix;

      if (col == 0 && row == 0) {
        // initialize with the potentially adjusted value of pix.
        min_x = pix[0];
        max_x = pix[0];
        min_y = pix[1];
        max_y = pix[1];
      }

      // Find the expanded (but tamed) image bounds
      if (pix[0] < min_x)
        min_x = pix[0];
      if (pix[0] > max_x)
        max_x = pix[0];
      if (pix[1] < min_y)
        min_y = pix[1];
      if (pix[1] > max_y)
        max_y = pix[1];
    }
  }

  // Convert the bounds to int
  min_x = floor(min_x); max_x = ceil(max_x);
  min_y = floor(min_y); max_y = ceil(max_y);

  // Ensure that the expanded image is not smaller than the old one,
  // to make the logic simpler
  if (min_x > 0)
    min_x = 0;
  if (max_x < img_cols)
    max_x = img_cols;
  if (min_y > 0)
    min_y = 0;
  if (max_y < img_rows)
    max_y = img_rows;

  // Convert the bounds to what cv::copyMakeBorder() will expect
  int border_top  = -min_y, border_bottom = max_y - img_rows;
  int border_left = -min_x, border_right  = max_x - img_cols;

  // Adjust the remapping function to the expanded image.
  // Now all its values will be within bounds of that image.
  for (int col = 0; col < floating_remap.cols; col++) {
    for (int row = 0; row < floating_remap.rows; row++) {
      cv::Vec2f pix = floating_remap.at<cv::Vec2f>(row, col);
      pix[0] += border_left;
      pix[1] += border_top;
      floating_remap.at<cv::Vec2f>(row, col) = pix;
    }
  }

  // Convert the map for speed
  cv::convertMaps(floating_remap, cv::Mat(), fixed_map, interp_map, CV_16SC2);

  Eigen::Vector2i dist_size(round(FLAGS_scale*cam_params.GetDistortedSize()[0]),
                            round(FLAGS_scale*cam_params.GetDistortedSize()[1]));
  Eigen::Vector2i undist_size(round(FLAGS_scale*cam_params.GetUndistortedSize()[0]),
                              round(FLAGS_scale*cam_params.GetUndistortedSize()[1]));
  double focal_length            = FLAGS_scale*cam_params.GetFocalLength();
  Eigen::Vector2d optical_center = FLAGS_scale*cam_params.GetUndistortedHalfSize();

  // Handle the cropping
  cv::Rect cropROI;
  if (!FLAGS_undistorted_crop_win.empty()) {
    std::vector<double> vals;
    ff_common::parseStr(FLAGS_undistorted_crop_win, vals);
    if (vals.size() < 2)
      LOG(FATAL) << "Could not parse --undistorted_crop_win.";

    int widx = vals[0];
    int widy = vals[1];

    // A couple of sanity checks
    if (widx % 2 != 0 || widy % 2 != 0 )
      LOG(FATAL) << "The cropped undistorted image dimensions must be even.";
    if (undist_size[0] % 2 != 0 || undist_size[1] % 2 != 0 )
      LOG(FATAL) << "The undistorted image dimensions must be even.";

    // Ensure that the crop window is within the image bounds
    int startx = std::max((undist_size[0] - widx)/2, 0);
    int starty = std::max((undist_size[1] - widy)/2, 0);
    widx = std::min(widx, undist_size[0] - startx);
    widy = std::min(widy, undist_size[1] - starty);

    // Update these quantities
    undist_size[0]     = widx;
    undist_size[1]     = widy;
    optical_center[0] -= startx;
    optical_center[1] -= starty;

    cropROI = cv::Rect(startx, starty, widx, widy);
    if (cropROI.empty())
      LOG(FATAL) << "Empty crop region.";

    std::cout << "Undistorted crop region: " << cropROI << std::endl;
  }

  for (size_t i = 0; i < images.size(); i++) {
    std::string filename(images[i]);

    cv::Mat image = cv::imread(filename, cv::IMREAD_UNCHANGED);

    if (FLAGS_histogram_equalization) {
      cv::Mat tmp_image;
      cv::equalizeHist(image, tmp_image);
      image = tmp_image;
    }

    // Ensure that image dimensions are as expected
    if (image.rows != img_rows || image.cols != img_cols)
      LOG(FATAL) << "The input images have wrong dimensions.";

    // Expand the image before interpolating into it
    cv::Scalar paddingColor = 0;
    cv::Mat expanded_image;
    cv::copyMakeBorder(image, expanded_image, border_top, border_bottom,
                       border_left, border_right,
                       cv::BORDER_CONSTANT, paddingColor);

    // Undistort it
    cv::Mat undist_image;
    cv::remap(expanded_image, undist_image, fixed_map, interp_map, cv::INTER_LINEAR);

    // Crop, if desired
    if (!cropROI.empty()) {
      cv::Mat cropped_image;
      undist_image(cropROI).copyTo(cropped_image);  // without copyTo it is a shallow copy
      undist_image = cropped_image;  // this makes a shallow copy
    }

    // The output file name
    std::string undist_file;
    if (!undist_images.empty()) {
      // Was specified via a list
      undist_file = undist_images[i];
    } else if (!FLAGS_output_directory.empty()) {
      // A separate output directory was specified
      undist_file = FLAGS_output_directory + "/" + ff_common::basename(filename);
    } else {
      // Save in same directory with new name
      undist_file = filename.substr(0, filename.size() - 4) + "_undistort.jpg";
    }

    // Save to disk the undistorted image
    std::cout << "Writing: " << undist_file << std::endl;
    cv::Mat bgr_image;
    if (FLAGS_save_bgr && undist_image.channels() == 1) {
      // Convert from grayscale to color if needed
      #if (CV_VERSION_MAJOR >= 4)
        cvtColor(undist_image, bgr_image, cv::COLOR_GRAY2BGR);
      #else
        cvtColor(undist_image, bgr_image, CV_GRAY2BGR);
      #endif
      undist_image = bgr_image;
    }
    cv::imwrite(undist_file, undist_image);
  }

  // Write some very useful info
  std::cout << "Distorted image size:       " << dist_size.transpose()      << "\n";
  std::cout << "Undistorted image size:     " << undist_size.transpose()    << "\n";
  std::cout << "Focal length:               " << focal_length               << "\n";
  std::cout << "Undistorted optical center: " << optical_center.transpose() << "\n";

  std::string intrinsics_file = FLAGS_undistorted_intrinsics;
  if (intrinsics_file.empty() && !FLAGS_output_directory.empty())
    intrinsics_file = FLAGS_output_directory + "/undistorted_intrinsics.txt";

  if (!intrinsics_file.empty()) {
    std::cout << "Writing: " << intrinsics_file << std::endl;
    std::ofstream ofs(intrinsics_file.c_str());
    ofs.precision(17);
    ofs << "# Unidistored width and height, focal length, undistorted optical center\n";
    ofs << undist_size.transpose() << " " << focal_length << " "
        << optical_center.transpose() << "\n";
    ofs.close();
  }

  return 0;
}
