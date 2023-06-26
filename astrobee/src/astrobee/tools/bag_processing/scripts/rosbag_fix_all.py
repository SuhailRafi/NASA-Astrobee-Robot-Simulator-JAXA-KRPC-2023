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
Master script to apply all passes of processing needed to fix our legacy bag
files. The actual processing steps are found in Makefile.rosbag_fix_all.
"""

from __future__ import print_function

import argparse
import logging
import os
import sys


def dosys(cmd):
    logging.info(cmd)
    ret = os.system(cmd)
    if ret != 0:
        logging.warning("Command failed with return value %s\n" % ret)
    return ret


def rosbag_fix_all(inbag_paths_in, jobs, deserialize=False):
    this_folder = os.path.dirname(os.path.realpath(__file__))
    makefile = os.path.join(this_folder, "Makefile.rosbag_fix_all")
    inbag_paths = [p for p in inbag_paths_in if not ".fix_all" in p]
    skip_count = len(inbag_paths_in) - len(inbag_paths)
    if skip_count:
        logging.info(
            "Not trying to fix %d files that already end in .fix_all.bag", skip_count
        )
    outbag_paths = [os.path.splitext(p)[0] + ".fix_all.bag" for p in inbag_paths]
    outbag_paths_str = " ".join(outbag_paths)

    rosbag_verify_args = "-v"
    if deserialize:
        rosbag_verify_args += " -d"

    # "1>&2": redirect stdout to stderr to see make's command echo in rostest output
    ret = dosys(
        'ROSBAG_VERIFY_ARGS="%s" make -f%s -j%s %s 1>&2'
        % (rosbag_verify_args, makefile, jobs, outbag_paths_str)
    )

    logging.info("")
    logging.info("====================")
    if ret == 0:
        logging.info("Fixed bags:")
        for outbag_path in outbag_paths:
            logging.info("  %s", outbag_path)
    else:
        logging.warning("Not all bags were fixed successfully (see errors above).")
        logging.warning(
            "You can debug any failed output bags - ending in .fix_all_pre_check.bag"
        )
        logging.warning(
            "If you want to try again, clean first: rm *.fix_all_pre_check.bag"
        )

    return ret


class CustomFormatter(argparse.ArgumentDefaultsHelpFormatter):
    pass


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=CustomFormatter
    )
    parser.add_argument(
        "-j",
        "--jobs",
        help="specifies the number of jobs to run simultaneously",
        type=int,
        default=1,
    )
    parser.add_argument(
        "-d",
        "--deserialize",
        help="perform deserialization check on output bag (can be slow for large bags)",
        default=False,
        action="store_true",
    )
    parser.add_argument("inbag", nargs="+", help="input bag")

    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO, format="%(message)s")

    ret = rosbag_fix_all(args.inbag, args.jobs, deserialize=args.deserialize)

    # suppress confusing ROS message at exit
    logging.getLogger().setLevel(logging.WARN)

    sys.exit(0 if ret == 0 else 1)
