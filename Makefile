
ifeq ($(ARCH),)
    ARCH=x86
else
    CROSS_COMPILE?=$(ARCH)-linux-
endif
BUILD_DIR?=$(ARCH)/

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LD=$(CROSS_COMPILE)gcc


PROGRAMM:=$(BUILD_DIR)dvb-tune
SOURCES:=$(wildcard *.c)
CC_FLAGS:=-Wall -Wextra

ifeq ($(ARCH),sh4)
PRJROOT?=/opt/elecard/DSP/sdk830
BUILDROOT?=$(PRJROOT)/build_stb830_24
KDIR?=$(BUILDROOT)/packages/linux-sh4-2.6.32.57_stm24_V5.0
LINUXTV_DIR=$(BUILDROOT)/packages/buildroot/output_rootfs/build/media_build/linux

#CC_FLAGS+=-I$(KDIR)/include
CC_FLAGS+=-I$(LINUXTV_DIR)/include/uapi
endif
#ifeq ($(ARCH),x86)
#endif

all: $(BUILD_DIR) $(PROGRAMM)

ifneq ($(BUILD_DIR),)
$(BUILD_DIR):
	mkdir -p $@
endif

$(PROGRAMM): $(SOURCES)
	$(CC) -o $(PROGRAMM) $(CC_FLAGS) $(SOURCES)

clean:
	rm -f $(PROGRAMM)
