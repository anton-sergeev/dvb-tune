
ifneq ($(BUILD_TARGET),)
    BUILD_PREFIX:=$(BUILD_TARGET)/
    CROSS_COMPILE?=$(BUILD_TARGET)-linux-
    CC=$(CROSS_COMPILE)gcc
    AR=$(CROSS_COMPILE)ar
    LD=$(CROSS_COMPILE)gcc
endif

PRJROOT?=/opt/elecard/DSP/sdk830
BUILDROOT?=$(PRJROOT)/build_stb830_24
KDIR?=$(BUILDROOT)/packages/linux-sh4-2.6.32.57_stm24_V5.0
LINUXTV_DIR=$(BUILDROOT)/packages/buildroot/output_rootfs/build/media_build/linux


PROGRAMM:=$(BUILD_PREFIX)dvb-tune
SOURCES:=$(wildcard *.c)
CC_FLAGS:=-Wall -Wextra
#CC_FLAGS+=-I$(KDIR)/include
CC_FLAGS+=-I$(LINUXTV_DIR)/include -I$(LINUXTV_DIR)/include/uapi -DLINUXTV

all: $(BUILD_TARGET) $(PROGRAMM)

ifneq ($(BUILD_TARGET),)
$(BUILD_TARGET):
	mkdir -p $(BUILD_PREFIX)
endif

$(PROGRAMM): $(SOURCES)
	$(CC) -o $(PROGRAMM) $(CC_FLAGS) $(SOURCES)

clean:
	rm -f $(PROGRAMM)
