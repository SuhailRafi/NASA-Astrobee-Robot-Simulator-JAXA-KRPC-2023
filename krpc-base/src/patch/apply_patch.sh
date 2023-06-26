#!/bin/bash
SOURCE_PATH=/src/astrobee/src
PATCH_SRC_PATH=./src/astrobee
BUILD_PATH=/src/astrobee

while read patch_file; do
    patch ${SOURCE_PATH}/${patch_file} ${PATCH_SRC_PATH}/${patch_file}.patch
done < ./filelist.txt

cd $BUILD_PATH
catkin build -j4