# This will set up an Astrobee melodic docker container using the non-NASA install
# instructions.
# This image is the base, meaning that it contains all the installation context,
# but it doesn't copy or build the entire code.
# You must set the docker context to be the repository root directory

ARG UBUNTU_VERSION=20.04
ARG ROS_VERSION=noetic
ARG PYTHON='3'
ARG http_proxy
ARG https_proxy
ARG HTTP_PROXY
ARG HTTPS_PROXY
ARG baseImage
FROM ${baseImage:-nvidia/opengl:1.0-glvnd-runtime-ubuntu$UBUNTU_VERSION}
ARG ROS_VERSION
ARG PYTHON
ARG http_proxy
ARG https_proxy
ARG HTTP_PROXY
ARG HTTPS_PROXY

# try to suppress certain warnings during apt-get calls
ARG DEBIAN_FRONTEND=noninteractive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN { \
    echo 'Acquire::http:proxy "'${http_proxy}'";'; \
    echo 'Acquire::https:proxy "'${https_proxy}'";'; \
    } | tee /etc/apt/apt.conf

RUN apt-get update && apt-get install -y gnupg vim iputils-ping net-tools
RUN if [ "${http_proxy}" = "" ]; then \
        apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654; \
    else \
        apt-key adv --keyserver keyserver.ubuntu.com --keyserver-options http-proxy=${http_proxy} --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654; \
    fi;

# install of apt-utils suppresses bogus warnings later
RUN apt-get update \
  && apt-get install -y apt-utils 2>&1 | grep -v "debconf: delaying package configuration, since apt-utils is not installed" \
  && apt-get install -y \
  build-essential \
  git \
  lsb-release \
  sudo \
  wget \
  && rm -rf /var/lib/apt/lists/*

# Install ROS --------------------------------------------------------------------
COPY ./scripts/setup/*.sh /setup/astrobee/

# this command is expected to have output on stderr, so redirect to suppress warning
RUN /setup/astrobee/add_ros_repository.sh >/dev/null 2>&1

RUN apt-get update \
  && apt-get install -y \
  debhelper \
  libtinyxml-dev \
  ros-${ROS_VERSION}-desktop \
  python${PYTHON}-rosdep \
  && rm -rf /var/lib/apt/lists/*

# Install Astrobee----------------------------------------------------------------
COPY ./scripts/setup/debians /setup/astrobee/debians
RUN sed -i "s;<HTTP_PROXY>;$http_proxy;g" /setup/astrobee/debians/opencv/patches/OpenCVDownload.patch \
  && sed -i "s;<HTTPS_PROXY>;$https_proxy;g" /setup/astrobee/debians/opencv/patches/OpenCVDownload.patch

RUN if [ "${http_proxy}" != "" ]; then \
      export http_proxy=$http_proxy; \
      export HTTP_PROXY=$HTTP_PROXY; \
    fi; \
    if [ "${https_proxy}" != "" ]; then \
      export https_proxy=$https_proxy; \
      export HTTPS_PROXY=$HTTPS_PROXY; \
    fi; 
RUN apt-get update \
  && /setup/astrobee/debians/build_install_debians.sh \
  && rm -rf /var/lib/apt/lists/* \
  && rm -rf /setup/astrobee/debians

COPY ./scripts/setup/packages_*.lst /setup/astrobee/
# note apt-get update is run within the following shell script
RUN /setup/astrobee/install_desktop_packages.sh \
  && rm -rf /var/lib/apt/lists/*

#Add the entrypoint for docker
RUN echo "#!/bin/bash\nset -e\n\nsource \"/opt/ros/${ROS_VERSION}/setup.bash\"\nsource \"/src/astrobee/devel/setup.bash\"\nexport ASTROBEE_CONFIG_DIR=\"/src/astrobee/src/astrobee/config\"\nexec \"\$@\"" > /astrobee_init.sh && \
  chmod +x /astrobee_init.sh && \
   if [ "${https_proxy}" = "" ]; then \
     unset https_proxy; \
     unset HTTPS_PROXY; \
   fi; \
  rosdep init && \
  rosdep update 2>&1 | egrep -v 'as root|fix-permissions'

