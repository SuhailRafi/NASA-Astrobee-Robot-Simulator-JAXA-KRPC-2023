ARG baseImage
FROM ${baseImage:-krpc/astrobee:noetic}

ENV USER root

# install install rviz plugin.
RUN apt-get update \
    && apt-get install -y \
        ros-noetic-jsk-rviz-plugins jq \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Copy new files
COPY ./src/astrobee /src/astrobee/src

# Patch
COPY ./src/patch /tmp/patch
WORKDIR /tmp/patch
RUN bash apply_patch.sh
RUN rm -rf /tmp/patch

# Add new sudo user
ENV USERNAME astrobee
RUN useradd -m $USERNAME && \
        echo "$USERNAME:$USERNAME" | chpasswd && \
        usermod --shell /bin/bash $USERNAME && \
        usermod -aG sudo $USERNAME && \
        echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/$USERNAME && \
        chmod 0440 /etc/sudoers.d/$USERNAME && \
        # Replace 1000 with your user/group id
        usermod  --uid 1000 $USERNAME && \
        groupmod --gid 1000 $USERNAME

RUN chown -R astrobee:astrobee /src/astrobee/src/astrobee
RUN chown -R astrobee:astrobee /src/astrobee/src/description

WORKDIR /home/astrobee
SHELL ["/bin/bash", "-c"]
RUN echo "source /astrobee_init.sh" >> /home/astrobee/.bashrc
RUN echo "source /src/astrobee/devel/setup.bash" >> /home/astrobee/.bashrc
RUN echo "export ROS_IP=\$(getent hosts llp | awk '{ print \$1 }')" >> /home/astrobee/.bashrc
RUN echo "export ROS_MASTER_URI=http://\${ROS_IP}:11311" >> /home/astrobee/.bashrc
