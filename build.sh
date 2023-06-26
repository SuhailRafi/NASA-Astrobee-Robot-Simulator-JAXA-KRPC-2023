#!/bin/bash
git_tag=v0.16.6

# Check Variables -------------------------------------------------
while getopts g option
do
    case $option in
        g)  gui="gui"
    esac
done

root_dir=$(pwd)
astrobee_dir=${root_dir}/astrobee/src/astrobee
if [ ! -e ${astrobee_dir} ]; then
    sudo apt-get install build-essential git
    git clone https://github.com/nasa/astrobee.git \
        -b ${git_tag} \
        --depth 1 \
        ${astrobee_dir}
    cd ${astrobee_dir}
    git pull origin 4bfe9474097561b9965e89eed18688a788d12769
    git submodule update --init description/media
    cd -

    patch ./astrobee/src/astrobee/scripts/docker/build.sh ./patches/build.sh.patch
    patch ./astrobee/src/astrobee/scripts/docker/astrobee_base.Dockerfile ./patches/astrobee_base.Dockerfile.patch 
    patch ./astrobee/src/astrobee/scripts/docker/astrobee.Dockerfile ./patches/astrobee.Dockerfile.patch
    patch ./astrobee/src/astrobee/scripts/setup/install_desktop_packages.sh ./patches/install_desktop_packages.sh.patch
    patch ./astrobee/src/astrobee/scripts/setup/debians/build_install_debians.sh ./patches/build_install_debians.sh.patch
    cp ./patches/OpenCVDownload.patch ./astrobee/src/astrobee/scripts/setup/debians/opencv/patches
    echo -e "\nOpenCVDownload.patch" >> ./astrobee/src/astrobee/scripts/setup/debians/opencv/patches/series
fi

# no gui(default)
export baseImage="nvidia/opengl:1.0-glvnd-runtime-ubuntu20.04"
export astrobeeBaseTag="krpc/astrobee:base-v0.16.6-ubuntu20.04"
export astrobeeTag="krpc/astrobee:v0.16.6-ubuntu20.04"
export krpcBaseTag="krpc/astrobee_sim-base:4.0.0"

if [ -n "${gui}" ]; then
    # with gui.
    echo "Build Image with GUI."
    export baseImage="dorowu/ubuntu-desktop-lxde-vnc:focal"
    export astrobeeBaseTag="krpc/astrobee:base-noetic-with-gui"
    export astrobeeTag="krpc/astrobee:noetic-with-gui"
    export krpcBaseTag="krpc/astrobee_sim-base-with-gui:4.0.0"
fi

bash ./astrobee/src/astrobee/scripts/docker/build.sh
docker build ./krpc-base \
        -t ${krpcBaseTag} \
        --build-arg baseImage=${astrobeeTag} \
        --build-arg HTTP_PROXY=${HTTP_PROXY} \
        --build-arg HTTPS_PROXY=${HTTPS_PROXY} \
        --build-arg http_proxy=${http_proxy} \
        --build-arg https_proxy=${https_proxy}
