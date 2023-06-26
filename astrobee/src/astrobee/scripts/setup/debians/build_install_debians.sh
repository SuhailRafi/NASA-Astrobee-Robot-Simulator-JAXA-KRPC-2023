#/bin/bash -e
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
#
# Install the dependencies needed for the debians. Build and install flight
# software debians.
set -e

DEBIAN_LOC=$(dirname "$(readlink -f "$0")")

sudo -E apt-get install -y devscripts equivs libproj-dev

# delete old debians (-f avoids 'no such file' warning on first run)
rm -f *_amd64.deb
DIST=`cat /etc/os-release | grep -oP "(?<=VERSION_CODENAME=).*"`

if [ "$DIST" != "xenial" ]; then
  echo "Ubuntu 16 not detected"

  # opencv
  cd ${DEBIAN_LOC}/opencv
  sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
  cd ${DEBIAN_LOC}
  ./build_opencv.sh || exit 1
  mv ${DEBIAN_LOC}/libopencv/libopencv*_amd64.deb .
  sudo -E dpkg -i libopencv*_amd64.deb || exit 1

  # alvar
  cp ${DEBIAN_LOC}/files/alvar_rules ${DEBIAN_LOC}/alvar/rules
  cp ${DEBIAN_LOC}/files/alvar_control ${DEBIAN_LOC}/alvar/control
  cp ${DEBIAN_LOC}/files/alvar_changelog ${DEBIAN_LOC}/alvar/changelog
  # dlib
  cp ${DEBIAN_LOC}/files/dlib_rules ${DEBIAN_LOC}/dlib/rules
  cp ${DEBIAN_LOC}/files/dlib_control ${DEBIAN_LOC}/dlib/control
  cp ${DEBIAN_LOC}/files/dlib_changelog ${DEBIAN_LOC}/dlib/changelog
  # dbow2
  cp ${DEBIAN_LOC}/files/dbow2_rules ${DEBIAN_LOC}/dbow2/rules
  cp ${DEBIAN_LOC}/files/dbow2_control ${DEBIAN_LOC}/dbow2/control
  cp ${DEBIAN_LOC}/files/dbow2_changelog ${DEBIAN_LOC}/dbow2/changelog
  # gtsam
  cp ${DEBIAN_LOC}/files/gtsam_changelog ${DEBIAN_LOC}/gtsam/changelog
fi

if [ "$DIST" = "bionic" ]; then
  echo "Ubuntu 18 detected"
   # jps3d
  sudo -E apt-get install -y libvtk6.3 libboost-filesystem1.62.0 libboost-system1.62.0
  cp ${DEBIAN_LOC}/files/jps3d_changelog ${DEBIAN_LOC}/jps3d/changelog
elif [ "$DIST" = "focal" ]; then
  echo "Ubuntu 20 detected"
  #jps3d
  sudo -E apt-get install -y libvtk7.1p libboost-filesystem1.71.0 libboost-system1.71.0
  cp ${DEBIAN_LOC}/files/jps3d_changelog ${DEBIAN_LOC}/jps3d/changelog
fi

# alvar
cd ${DEBIAN_LOC}/alvar
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_alvar.sh || exit 1
sudo -E dpkg -i libalvar*_amd64.deb || exit 1

# dlib
cd ${DEBIAN_LOC}/dlib
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_dlib.sh || exit 1
sudo -E dpkg -i libdbowdlib*_amd64.deb || exit 1

# dbow2
cd ${DEBIAN_LOC}/dbow2
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_dbow2.sh || exit 1
sudo -E dpkg -i libdbow*_amd64.deb || exit 1

# gtsam
cd ${DEBIAN_LOC}/gtsam
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_gtsam.sh || exit 1
sudo -E dpkg -i libgtsam*_amd64.deb || exit 1

# decomputil
cd ${DEBIAN_LOC}/decomputil
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_decomputil.sh || exit 1
sudo -E dpkg -i libdecomputil*_amd64.deb || exit 1

# jps3d
cd ${DEBIAN_LOC}/jps3d
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_jps3d.sh || exit 1
sudo -E dpkg -i libjps3d*_amd64.deb || exit 1

# openmvg
cd ${DEBIAN_LOC}/openmvg
sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
cd ${DEBIAN_LOC}
./build_openmvg.sh || exit 1
sudo -E dpkg -i libopenmvg*_amd64.deb || exit 1

REQUIRED_PKG="rti-dev"
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG 2>&1 | grep "install ok installed" || true)
echo Checking for $REQUIRED_PKG: $PKG_OK
if [ "install ok installed" = "$PKG_OK" ]; then
  echo "$REQUIRED_PKG exists. Setting up miro and soracore."

  # miro
  cd ${DEBIAN_LOC}/miro
  sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
  cd ${DEBIAN_LOC}
  ./build_miro.sh || exit 1
  sudo -E dpkg -i libmiro*_amd64.deb || exit 1

  # soracore
  cd ${DEBIAN_LOC}/soracore
  sudo -E mk-build-deps -i -r -t "apt-get --no-install-recommends -y" control
  cd ${DEBIAN_LOC}
  ./build_soracore.sh || exit 1
  sudo -E dpkg -i libsoracore*_amd64.deb || exit 1

fi

# Rename debians
for file in *.deb; do mv "$file" "${file%.deb}_${DIST}.deb"; done;
