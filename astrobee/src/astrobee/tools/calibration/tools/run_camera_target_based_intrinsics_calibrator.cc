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

#include <calibration/camera_target_based_intrinsics_calibrator_params.h>
#include <calibration/camera_target_based_intrinsics_calibrator.h>
#include <calibration/parameter_reader.h>
#include <ff_common/init.h>
#include <ff_common/utils.h>
#include <localization_common/image_correspondences.h>
#include <localization_common/logger.h>
#include <localization_common/utilities.h>
#include <vision_common/fov_distorter.h>
#include <vision_common/rad_distorter.h>
#include <vision_common/radtan_distorter.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace ca = calibration;
namespace lc = localization_common;
namespace vc = vision_common;
namespace po = boost::program_options;

lc::ImageCorrespondences LoadTargetMatches(const std::string& match_file) {
  std::vector<Eigen::Vector2d> image_points;
  std::vector<Eigen::Vector3d> points_3d;
  std::cout << "Reading: " << match_file << std::endl;
  std::ifstream cr(match_file.c_str());
  std::string line;
  while (std::getline(cr, line)) {
    double val;
    std::vector<double> vals;
    std::istringstream is(line);
    while (is >> val) vals.push_back(val);
    if (vals.size() != 5) LogFatal("Each line of " << match_file << " must have 5 entries.");
    points_3d.emplace_back(Eigen::Vector3d(vals[1], vals[2], 0));
    image_points.emplace_back(Eigen::Vector2d(vals[3], vals[4]));
  }

  return lc::ImageCorrespondences(image_points, points_3d);
}

std::vector<lc::ImageCorrespondences> LoadAllTargetMatches(const std::string& corners_directory,
                                                           const int max_num_match_sets) {
  std::vector<std::string> corners_files;
  std::vector<lc::ImageCorrespondences> all_matches;
  ff_common::ListFiles(corners_directory, "txt", &corners_files);
  int i = 0;
  for (const auto& corners_file : corners_files) {
    const auto& matches = LoadTargetMatches(corners_file);
    if (matches.size() < 4) {
      LogError("Too few matches, only " << matches.size() << ".");
      continue;
    }
    all_matches.emplace_back(matches);
    if (++i > max_num_match_sets) break;
  }
  return all_matches;
}

void WriteCalibrationResultsToFile(const Eigen::Vector2d& focal_lengths, const Eigen::Vector2d& principal_points,
                                   const Eigen::VectorXd& distortion, const std::string& output_filename) {
  std::ofstream output_file;
  output_file.open(output_filename);
  output_file << focal_lengths[0] << " " << focal_lengths[1] << std::endl;
  output_file << principal_points[0] << " " << principal_points[1] << std::endl;
  int i = 0;
  for (; i < distortion.size() - 1; ++i) {
    output_file << distortion[i] << " ";
  }
  output_file << distortion[i];
  output_file.close();
}

template <typename DISTORTER>
bool Calibrate(const ca::RunCalibratorParams& params, const std::vector<lc::ImageCorrespondences>& target_matches,
               const std::string& output_file) {
  ca::CameraTargetBasedIntrinsicsCalibrator<DISTORTER> calibrator(params.camera_target_based_intrinsics_calibrator);
  ca::StateParameters initial_state_parameters;
  initial_state_parameters.focal_lengths = params.camera_params->GetFocalVector();
  initial_state_parameters.principal_points = params.camera_params->GetOpticalOffset();
  initial_state_parameters.distortion = params.camera_params->GetDistortion();
  ca::StateParameters calibrated_state_parameters;
  ca::StateParametersCovariances covariances;
  const bool success = calibrator.EstimateInitialTargetPosesAndCalibrate(target_matches, initial_state_parameters,
                                                                         calibrated_state_parameters, covariances);
  if (!success) {
    LogError("Calibrate: Calibration failed.");
    return false;
  }
  if (params.camera_target_based_intrinsics_calibrator.calibrate_focal_lengths) {
    LogInfo("initial focal lengths: " << std::endl << initial_state_parameters.focal_lengths.matrix());
    LogInfo("calibrated focal lengths: " << std::endl << calibrated_state_parameters.focal_lengths.matrix());
    LogInfo("calibrated focal length covariance: " << std::endl << covariances.focal_lengths.matrix());
  }
  if (params.camera_target_based_intrinsics_calibrator.calibrate_principal_points) {
    LogInfo("initial principal points: " << std::endl << initial_state_parameters.principal_points.matrix());
    LogInfo("calibrated principal points: " << std::endl << calibrated_state_parameters.principal_points.matrix());
    LogInfo("calibrated principal points covariance: " << std::endl << covariances.principal_points.matrix());
  }
  if (params.camera_target_based_intrinsics_calibrator.calibrate_distortion) {
    LogInfo("initial distortion: " << std::endl << initial_state_parameters.distortion.matrix());
    LogInfo("calibrated distortion: " << std::endl << calibrated_state_parameters.distortion.matrix());
    LogInfo("calibrated distortion covariance: " << std::endl << covariances.distortion.matrix());
  }
  WriteCalibrationResultsToFile(calibrated_state_parameters.focal_lengths, calibrated_state_parameters.principal_points,
                                calibrated_state_parameters.distortion, output_file);
  return true;
}

int main(int argc, char** argv) {
  std::string robot_config_file;
  std::string world;
  std::string output_file;
  po::options_description desc("Calibrates camera intrinsics using target detections.");
  desc.add_options()("help,h", "produce help message")(
    "corners-directory", po::value<std::string>()->required(),
    "Directory containing target detections to use for calibration.")(
    "config-path,c", po::value<std::string>()->required(), "Config path")(
    "robot-config-file,r", po::value<std::string>(&robot_config_file)->default_value("config/robots/bumble.config"),
    "Robot config file")("world,w", po::value<std::string>(&world)->default_value("iss"), "World name")(
    "output-file,o", po::value<std::string>(&output_file)->default_value("calibrated_params.txt"),
    "File where calibrated parameters are saved.");
  po::positional_options_description p;
  p.add("corners-directory", 1);
  p.add("config-path", 1);
  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    if (vm.count("help") || (argc <= 1)) {
      std::cout << desc << "\n";
      return 1;
    }
    po::notify(vm);
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  const std::string corners_directory = vm["corners-directory"].as<std::string>();
  const std::string config_path = vm["config-path"].as<std::string>();

  // Only pass program name to free flyer so that boost command line options
  // are ignored when parsing gflags.
  int ff_argc = 1;
  ff_common::InitFreeFlyerApplication(&ff_argc, &argv);

  lc::SetEnvironmentConfigs(config_path, world, robot_config_file);
  config_reader::ConfigReader config;
  config.AddFile("geometry.config");
  config.AddFile("cameras.config");
  config.AddFile("tools/camera_target_based_intrinsics_calibrator.config");
  if (!config.ReadFiles()) {
    LogFatal("Failed to read config files.");
  }
  ca::RunCalibratorParams params;
  LoadRunCalibratorParams(config, params);

  std::vector<lc::ImageCorrespondences> target_matches;
  if (!boost::filesystem::is_directory(corners_directory)) {
    LogFatal("Corners directory " << corners_directory << " not found.");
  }
  LogInfo("Calibrating " << robot_config_file << ", camera: " << params.camera_name);
  target_matches =
    LoadAllTargetMatches(corners_directory, params.camera_target_based_intrinsics_calibrator.max_num_match_sets);
  LogInfo("Number of target match sets: " << target_matches.size());
  if (params.distortion_type == "fov") {
    Calibrate<vc::FovDistorter>(params, target_matches, output_file);
  } else if (params.distortion_type == "rad") {
    Calibrate<vc::RadDistorter>(params, target_matches, output_file);
  } else if (params.distortion_type == "radtan") {
    Calibrate<vc::RadTanDistorter>(params, target_matches, output_file);
  } else {
    LogFatal("Invalid distortion type provided.");
  }
}
