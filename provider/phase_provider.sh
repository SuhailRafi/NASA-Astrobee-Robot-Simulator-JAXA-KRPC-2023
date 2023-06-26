#! /bin/bash
# Only can use on GUI.

source /opt/ros/noetic/setup.bash

gnome-terminal \
    --tab -- /bin/bash -c "source /opt/ros/noetic/setup.bash;python3 ./phase_provider.py"
