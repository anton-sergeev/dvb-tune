#!/bin/bash
#Build for sh4:
PRJROOT=${PRJROOT:-/opt/elecard/DSP/sdk830}
BUILDROOT=${BUILDROOT:-$PRJROOT/build_stb830_24}

#export PATH=$PATH:/opt/STM/STLinux-2.4/devkit/sh4/bin
#export ARCH=sh4
#make


SRCROOT="$(readlink -f $(dirname ${BASH_SOURCE[0]:-.}))"
CROSS_COMPILE_PATH=/opt/STM/STLinux-2.4/devkit/sh4/bin


cmake \
  -DCMAKE_C_COMPILER=$CROSS_COMPILE_PATH/sh4-linux-gcc \
  -DCMAKE_C_FLAGS="-std=c99 -D_BSD_SOURCE" \
  -S "$SRCROOT" -B cmake_build/sh4
cmake --build cmake_build/sh4


# echo "Update $BUILDROOT/rootfs_nfs/serga/dvb/dvb-tune"
# cp $ARCH/dvb-tune $BUILDROOT/rootfs_nfs/serga/dvb/


