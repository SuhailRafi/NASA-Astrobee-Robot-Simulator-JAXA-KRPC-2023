#!/bin/bash

##############################
# Static values
#

rootdir=$(dirname "$(readlink -f "$0")")
cd $rootdir
bootstrap_dir=${rootdir}/bootstrap
provider_dir=${rootdir}/provider

OBJECT_NAME_ARRAY=("qr" "target1" "target2" "target3" "target4" "target5" "target6")

XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth
touch $XAUTH
xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -

##############################
# Run Simulator
#

ROS_IP=$(getent hosts llp | awk '{ print $1 }')

docker run -it --rm --name astrobee \
        --volume=$XSOCK:$XSOCK:rw \
        --volume=$XAUTH:$XAUTH:rw \
        --volume=${bootstrap_dir}:/tmp/bootstrap \
	--volume=${provider_dir}:/tmp/provider \
        --env="XAUTHORITY=${XAUTH}" \
        --env="DISPLAY" \
        --env="ROS_MASTER_URI=http://${ROS_IP}:11311" \
        --env="ROS_HOSTNAME=${ROS_IP}" \
        --user="astrobee" \
        --privileged \
        --network=host \
        krpc/astrobee_sim-base:4.0.0 \
        /astrobee_init.sh bash /tmp/bootstrap/run.sh

