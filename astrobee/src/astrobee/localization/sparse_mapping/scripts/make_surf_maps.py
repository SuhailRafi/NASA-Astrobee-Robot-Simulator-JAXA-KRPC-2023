#!/usr/bin/python
#
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
Makes surf maps for the provided bagfiles. See make_surf_map.py script for more
details on surf map creation.
"""

import argparse
import csv
import glob
import itertools
import multiprocessing
import os
import sys

import localization_common.utilities as lu

# Import as to avoid same name as function in this script
import make_surf_map as mm


# Add traceback so errors are forwarded, otherwise
# some errors are suppressed due to the multiprocessing
# library call
@lu.full_traceback
def make_surf_map(bagfile, world, robot_name):
    mm.make_surf_map(bagfile, world, robot_name)


# Helper that unpacks arguments and calls original function
# Aides running jobs in parallel as pool only supports
# passing a single argument to workers
def make_surf_map_helper(zipped_vals):
    return make_surf_map(*zipped_vals)


def make_surf_maps(bags, world, robot_name, num_processes):
    pool = multiprocessing.Pool(num_processes)
    pool.map(
        make_surf_map_helper,
        list(zip(bags, itertools.repeat(world), itertools.repeat(robot_name))),
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--bags",
        nargs="*",
        help="List of bags to make maps for. If none provided, all bags in the current directory are used.",
    )
    parser.add_argument("-w", "--world", default="iss")
    parser.add_argument("-r", "--robot-name", default="bumble")
    parser.add_argument(
        "-p",
        "--num-processes",
        type=int,
        default=10,
        help="Number of concurrent processes to run, where each map creation job is assigned to one process.",
    )
    args = parser.parse_args()
    bags = args.bags if args.bags is not None else glob.glob("*.bag")
    make_surf_maps(bags, args.world, args.robot_name, args.num_processes)
