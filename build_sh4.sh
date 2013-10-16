#!/bin/bash
#Build for sh4:
PRJROOT=${PRJROOT:-/opt/elecard/DSP/sdk830}
BUILDROOT=${BUILDROOT:-$PRJROOT/build_stb830_24}

export PATH=$PATH:/opt/STM/STLinux-2.4/devkit/sh4/bin
export ARCH=sh4
make
cp $ARCH/dvb-tune $BUILDROOT/rootfs_nfs/serga/dvb/
