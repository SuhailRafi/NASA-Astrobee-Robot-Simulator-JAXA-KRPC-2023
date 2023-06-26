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
Averages results from all *stats.csv files in a directory (including subdirectories) and 
saves results to a new file averaged_results.csv.
"""

import argparse
import os

import pandas as pd


def combined_results(csv_files):
    dataframes = [pd.read_csv(file, header=None) for file in csv_files]
    if not dataframes:
        print("Failed to create dataframes")
        exit()
    names = dataframes[0].iloc[:, 0]
    combined_dataframes = pd.DataFrame(None, None, names)
    for dataframe in dataframes:
        trimmed_dataframe = pd.DataFrame(
            dataframe.transpose().values[1:2], columns=names
        )
        combined_dataframes = combined_dataframes.append(
            trimmed_dataframe, ignore_index=True
        )
    return combined_dataframes


def average_results(directory, csv_files):
    combined_dataframes = combined_results(csv_files)
    names = combined_dataframes.columns
    mean_dataframe = pd.DataFrame()
    for name in names:
        mean_dataframe[name] = [combined_dataframes[name].mean()]
    averaged_results_file = os.path.join(directory, "averaged_results.csv")
    mean_dataframe.to_csv(averaged_results_file, index=False)
