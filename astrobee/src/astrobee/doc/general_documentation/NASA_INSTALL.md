\page install-NASA Install for NASA users

# Usage instructions for NASA users

Install the 64-bit version of
[Ubuntu16.04](http://releases.ubuntu.com/16.04) on a host machine, and
make sure that you can checkout and build code.

    sudo apt-get install build-essential git

*Note: Please ensure you install the 64-bit version of Ubuntu. We do not
support running Astrobee Robot Software on 32-bit systems.*

## Computer setup

### Username

If you are using a VM with a username that does not match your NDC username,
please configure the following variable:

    export NDC_USERNAME=your_ndc_username


### Access to the Astrobee Debian server

The custom debian packages are currently distributed by the
`astrobee.ndc.nasa.gov server`. This server is currently on the ARC TI
private network. It is *critical* to be able to reach this server to
install the pre-built custom debian. If none of the solutions below
allow you to reach `astrobee.ndc.nasa.gov`, you can use the
instructions on how to build the Debian dependencies manually
following the INSTALL.md instructions (Dependencies section).

#### If on the ARC TI private network

This is the typical case for all wired computers in ARC TI, and simplifies
your life greatly.

Verify that you are in this situation with the command below should succeed
(remove the Release.gpg file after being fetched).

    wget -v http://astrobee.ndc.nasa.gov/software/dists/xenial/Release.gpg

Before running the scripts in `scripts/setup` below, set this variable:

    export NO_TUNNEL=1

#### If not on the ARC TI private network

If you are outside the NASA ARC private network, there are two options to
reach `astrobee.ndc.nasa.gov`:

  1. Use VPN to act like if you were inside the ARC TI private network and 
     obtain the correct kerberos credentials inside the VM with the following
     command (note the capitalization): `kinit $NDC_USERNAME@NDC.NASA.GOV`
  2. setup your `.ssh/config` to do ssh forwarding. A tutorial on this method
  is available at: https://babelfish.arc.nasa.gov/trac/freeflyer/wiki/SSHSetup

For either solution, please verify that you can SSH to `m.ndc.nasa.gov` without
entering your password (`m` is used to tunnel to `astrobee.ndc.nasa.gov`):

   `ssh $NDC_USERNAME@m.ndc.nasa.gov`
   
The command should succeed without entering your password. Once this is verified,
exit this session on `m` with `<ctrl>+D`.

- These notes apply to `install_desktop_16.04_packages.sh` and `make_xenial.sh`


### Checkout the project source code

At this point you need to decide where you'd like to put the source code
(`ASTROBEE_WS`) on your machine:

    export ASTROBEE_WS=$HOME/astrobee

First, clone the flight software repository:

    git clone https://github.com/nasa/astrobee.git --branch develop $ASTROBEE_WS/src
    git submodule update --init --depth 1 description/media
    git submodule update --init --depth 1 submodules/platform

You can either choose which optional submodules to clone and log depth with:

    git submodule update --init --depth 1 submodules/android
    git submodule update --init --depth 1 submodules/avionics

Or checkout all the submodules as:

    git submodule update --init --depth 1

(Note: re-enter your username and password for every submodule that is cloned)
The android module is necessary for guest science code; the avionics and platform
module is used when cross-compiling to test on the robot hardware.

### Dependencies
Install dependencies:

    pushd $ASTROBEE_WS
    cd src/scripts/setup
    ./add_local_repository.sh
    ./add_ros_repository.sh
    ./install_desktop_packages.sh
    popd

#### Extra options to install the dependencies

- If you do not want to configure your `.ssh/config` to just get the
dependencies, you can use the `NDC_USERNAME` variable.
- By default, the custom debians are installed in `$SOURCE_PATH/.astrobee_deb`.
If you prefer to install them at a different location, you can use the
`ARS_DEB_DIR` variable.

    export NDC_USERNAME=jdoe
    export ARS_DEB_DIR=$HOME/astrobee_debs
    ./add_local_repository.sh

### Cross-compile setup

If you are planning to compile code to run on the robot hardware, you will need
to install a cross-compile chroot and toolchain. Select two directories for
these:

    export ARMHF_CHROOT_DIR=$HOME/arm_cross/rootfs
    export ARMHF_TOOLCHAIN=$HOME/arm_cross/toolchain/gcc

Append these lines to your .bashrc file, as you will need these two variables
every time you cross compile.

Next, download the cross toolchain and install the chroot:

    mkdir -p $ARMHF_TOOLCHAIN
    cd $HOME/arm_cross
    $ASTROBEE_WS/src/submodules/platform/fetch_toolchain.sh
    $ASTROBEE_WS/src/submodules/platform/rootfs/make_xenial.sh dev $ARMHF_CHROOT_DIR

*Note: The last script shown above needs the packages `qemu-user-static` (not
`qemu-arm-static`) and `multistrap` to be installed (can be installed through apt).*

## Configuring the build

At this point you need to decide whether you'd like to compile natively
[`native`] (run code against a simulator) or for an ARM target [`armhf`] (run
the code on the robot itself). Please skip to the relevant subsection.

### Note for both builds setup

When compiling, the `$WORKSPACE_PATH` defines where the `devel`, `build`, `logs` and
`install` directories are created. If you want to customize the `install` path then the
`$INSTALL_PATH` can be defined. By default, the configure script uses the following paths:

  - native build path (WORKSPACE_PATH): `$ASTROBEE_WS`
  - native install path (INSTALL_PATH): `$ASTROBEE_WS/install`
  - armhf build path (WORKSPACE_PATH):  `$ASTROBEE_WS/armhf`
  - armhf install path (INSTALL_PATH):  `$ASTROBEE_WS/armhf/opt/astrobee`

You should set these values in your shell. 

If you are satisfied with these paths, you can invoke the `configure.sh` without
the `-p` and `-w` options. For the simplicity of the instructions below,
we assume that `$WORKSPACE_PATH` and `$INSTALL_PATH` contain the location of the
build and install path for either `native` or `armhf` platforms.

### Native build

The configure script prepares your build directory for compiling the code. Note
that `configure.sh` is simply a wrapper around CMake that provides an easy way
of turning on and off options. To see which options are supported, simply run
`configure.sh -h`.

    pushd $ASTROBEE_WS
    ./src/scripts/configure.sh -l
    source ~/.bashrc
    popd

The configure script modifies your ``.bashrc`` to source ``setup.bash`` for 
the current ROS distribution and to set CMAKE_PREFIX_PATH. It is suggested
to examine it and see if all changes were made correctly.

If you want to explicitly specify the workspace and/or install directories, use
instead:

    ./scripts/configure.sh -l -p $INSTALL_PATH -w $WORKSPACE_PATH

*Note: If a workspace is specified but not an explicit install distectory,
install location will be $WORKSPACE_PATH/install.*

### Cross-compile build

Cross compiling for the robot follows the same process, except the configure
script takes a `-a` flag instead of `-l`.

    pushd $ASTROBEE_WS
    ./src/scripts/configure.sh -a
    popd

Or with explicit build and install paths:

    ./scripts/configure.sh -a -p $INSTALL_PATH -w $WORKSPACE_PATH

*Warning: `$INSTALL_PATH` and `$WORKSPACE_PATH` used for cross compiling HAVE to be
different than the paths for native build! See above for the default values 
for these.*

## Building the code

To build, run `catkin build` in the `$WORKSPACE_PATH`. Note that depending on your host
machine, this might take in the order of tens of minutes to complete the first
time round. Future builds will be faster, as only changes to the code are
rebuilt, and not the entire code base.

    pushd $ASTROBEE_WS
    catkin build
    popd

## Switching build profiles

To alternate between native and armhf profiles:

    catkin profile set native
    catkin profile set armhf

## Running a simulation

In order to run a simulation you must have build natively. You will need to
first setup your environment, so that ROS knows about the new packages provided
by Astrobee flight software:

    pushd $ASTROBEE_WS
    source devel/setup.bash
    popd

After this command has completed, you should be able to run a simulator from any
directory in your Linux filesystem. So, for example, to start a simulation of a
single Astrobee in the Granite Lab, run the following:

    roslaunch astrobee sim.launch

This command tells ROS to look for the `sim.launch` file provided by the
`astrobee` package, and use roslaunch to run it. Internally, ROS maintains a
cache of information about package locations, libraries and executables. If you
find that the above command doesn't work, try rebuilding the cache:

    rospack profile

For more information on running the simulator and moving the robot, please see the \ref sim-readme.

## Running the code on a real robot

In order to do this, you will need to have followed the cross-compile build
instructions. Once the code has been built, it also installs the code to
a singular location. CMake remembers what `$INSTALL_PATH` you specified, and
will copy all products into this directory.

Once the installation has completed, copy the install directory to the robot.
This script assumes that you are connected to the Astrobee network, as it uses
rsync to copy the install directory to `~/armhf` on the two processors. It 
takes the robot name as an argument. Here we use `p4d'.

    pushd $ASTROBEE_WS
    ./src/scripts/install_to_astrobee.sh $INSTALL_PATH p4d
    popd

Here, p4d is the name of the robot, which may be different in your case.

You are now ready to run the code. This code launches a visualization tool,
which starts the flight software as a background process.

    pushd $ASTROBEE_WS
    python ./src/tools/gnc_visualizer/scripts/visualizer --proto4
    popd

# Further information

Please refer to the [wiki](https://github.com/nasa/astrobee/wiki).
