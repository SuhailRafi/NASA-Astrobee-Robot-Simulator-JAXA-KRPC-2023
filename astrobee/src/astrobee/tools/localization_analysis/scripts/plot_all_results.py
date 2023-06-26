#!/usr/bin/env python
# Copyright (c) 2017, United States Government, as represented by the
# Administrator of the National Aeronautics and Space Administration.
#
# All rights reserved.
#
# The Astrobee platform is licensed under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
"""
Plots localization results for each bagfile in a directory.
Assumes no custom start and end times are provided and no special
groundtruth bags are available for each bagfile.  See 
plot_results.py for more details on results plotting.
"""


import argparse
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--output-dir",
        default="",
        help="Output directory where generated pdfs for bagfiles are saved.",
    )
    args = parser.parse_args()

    # Find bagfiles with bag prefix in current directory, fail if none found
    bag_names = [
        bag for bag in os.listdir(".") if os.path.isfile(bag) and bag.endswith(".bag")
    ]
    bag_names = sorted(set(bag_names))
    if len(bag_names) == 0:
        print("No bag files found")
        sys.exit()
    else:
        print(("Found " + str(len(bag_names)) + " bags."))

    for bag_name in bag_names:
        output_file_name = (
            os.path.splitext(os.path.basename(bag_name))[0] + "_output.pdf"
        )
        if args.output_dir:
            output_file_name = args.output_dir + "/" + output_file_name
        plot_command = (
            "rosrun localization_analysis plot_results.py "
            + bag_name
            + " --output-file "
            + output_file_name
        )
        os.system(plot_command)
