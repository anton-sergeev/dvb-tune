#!/bin/bash

#Build for sh4:
PRJROOT=${PRJROOT:-/opt/Module/serga}
BUILDROOT=${BUILDROOT:-$PRJROOT/build_arm_module}
export PATH=$PATH:$BUILDROOT/packages/output_rootfs/host/usr/bin

make ARCH=arm BUILD_DIR=arm_Module CROSS_COMPILE=arm-none-linux-gnueabi-
