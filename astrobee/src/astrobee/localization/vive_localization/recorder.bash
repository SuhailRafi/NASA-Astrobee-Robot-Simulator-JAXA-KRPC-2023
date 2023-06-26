#!/bin/bash

#
# Script to help record different data sets with consistent filenames
#

if [[ $# -lt 3 ]] 
then
  echo "Usage:"
  echo "  recorder.sh what procedure step [run [info]]"
  echo "    what:       type of data to record"
  echo "         pmc    record pmc related data"
  echo "         img    record data for the on orbit functional checkout"
  echo "         clf    record data for calibration of nav and haz cams (+imu)"
  echo "         cla    record data for calibration of dock and perch cams (+imu)"
  echo "         map    record data for map building"
  echo "         mob    record data for the mobility procedure"
  echo "    procedure:  procedure nickname"
  echo "    step:       step or section to record"
  echo "    run:        run number (default = 1)"
  echo "    info:       description for the manifest"
  exit -1
fi

what=$1
proc=$2
step=$3

if [[ $# -gt 3 ]]
then
  run=$4
else
  run="1"
fi

if [[ $# -gt 4 ]]
then
  info=$5
else
  info="n/a"
fi

source /res/astrobee.env
if [[ "$ASTROBEE_WORLD" == "granite" ]]
then 
  zone="UTC+8"
else
  zone="UTC"
fi

# Get a date that match where test is performed
today=`TZ=$zone date +%Y-%m-%d`

# Get the robot name
robot=`cat /etc/robotname`

# Make sure we have a place to write
dest=/data/bags/$robot/$today
mkdir -p $dest

# Craft a filename
start=`TZ=$zone date +%Y%m%d_%H%M`
name="${start}_proc-${proc}_step-${step}_run-${run}"
bagfile=$dest/$name.bag
echo $info > $dest/$name.txt

# ROS Core is running on the LLP
export ROS_MASTER_URI=http://${MASTER_IP:-llp}:11311

topics=""
include=""
exclude=""

case $what in
  pmc)
    topics="/hw/eps/housekeeping /hw/pmc/telemetry /hw/pmc/command"
    ;;
  img)
    topics="/hw/cam_nav /hw/cam_dock /hw/imu"
    include="-e /hw/eps/(.*)|/hw/speed_cam/(.*)|/hw/(.*)/points"
    ;;
  clf)
    topics="/hw/cam_nav /hw/depth_haz/extended/amplitude_int /hw/imu"
    ;;
  cla)
    topics="/hw/cam_dock /hw/depth_perch/extended/amplitude_int /hw/imu"
    ;;
  map)
    topics="/hw/cam_nav /hw/imu"
    ;;
  mob)
    include="-e /gnc/(.*)|/loc/(.*)|/mob/(.*)|/hw/imu|/hw/vive/(.*)"
    exclude="-x /mob/mapper/(.*)"
    ;;
  *)
    echo "invalid 'what' argument!"
    exit 1
  ;;
esac

echo "Starting to record $bagfile"
rosbag record $topics $include $exclude -O $bagfile
