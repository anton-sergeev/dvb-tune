#!/bin/bash

#Build for arm Module:
# PRJROOT=${PRJROOT:-/opt/Module/serga}
# BUILDROOT=${BUILDROOT:-$PRJROOT/build_arm_module}
# export PATH=$PATH:$BUILDROOT/packages/output_rootfs/host/usr/bin

SRCROOT="$(readlink -f $(dirname ${BASH_SOURCE[0]:-.}))"
CROSS_COMPILE_PATH=/opt/Module/serga/build_arm_module/packages/output_rootfs/host/usr/bin

#make ARCH=arm BUILD_DIR=arm_Module CROSS_COMPILE=arm-none-linux-gnueabi-
cmake \
  -DCMAKE_C_COMPILER=$CROSS_COMPILE_PATH/arm-none-linux-gnueabi-gcc \
  -DCMAKE_C_FLAGS="-std=c99 -D_BSD_SOURCE -DDISABLE_DVB_V5_STATS" \
  -S "$SRCROOT" -B cmake_build/arm_Module
cmake --build cmake_build/arm_Module


