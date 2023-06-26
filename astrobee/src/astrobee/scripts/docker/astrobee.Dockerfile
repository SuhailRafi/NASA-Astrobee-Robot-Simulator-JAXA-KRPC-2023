# This will set up an Astrobee melodic docker container using the non-NASA install instructions.
# This image builds on top of the base melodic image building the code.
# You must set the docker context to be the repository root directory

ARG UBUNTU_VERSION=20.04
ARG ROS_VERSION=noetic
ARG NUM_JOBS=1
ARG REMOTE=krpc
ARG TAG=krpc/astrobee:base-latest-ubuntu${UBUNTU_VERSION}
FROM ${TAG}
ARG UBUNTU_VERSION
ARG ROS_VERSION
ARG NUM_JOBS

COPY . /src/astrobee/src/
RUN . /opt/ros/${ROS_VERSION}/setup.sh \
	&& cd /src/astrobee \
	&& ./src/scripts/configure.sh -l -F -D -T \
	&& CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/src/astrobee/src/cmake \
	&& catkin build --no-status --force-color -j${NUM_JOBS}

COPY ./astrobee/resources /opt/astrobee/share/astrobee/resources
