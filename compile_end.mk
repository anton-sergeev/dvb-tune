# ****************************************************************************
# File Name   : compile_end.mk
# Copyright (C) 2013 Elecard Devices
# *****************************************************************************


OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
OBJECTS += $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(CXX_SOURCES))
OBJECTS_DIRS := $(sort $(BUILD_DIR)/ $(dir $(OBJECTS)))
DIRECTORIES += $(OBJECTS_DIRS)
PROGRAMM := $(BUILD_DIR)/$(PROGRAM_NAME)

cmd_files += $(wildcard $(foreach f,$(OBJECTS) $(PROGRAMM),$(call get_cmd_file,$(f))))
ifneq ($(cmd_files),)
  include $(cmd_files)
endif

$(OBJECTS): | $(OBJECTS_DIRS)

$(DIRECTORIES):
	$(Q)mkdir -p $@

$(PROGRAMM): $(OBJECTS) $(ADD_LIBS) $(DEPENDS_EXTRA) force
	$(call if_changed,ld_out_o)

build: $(PROGRAMM)

clean:
	rm -f $(ADD_LIBS) $(OBJECTS) $(cmd_files) $(PROGRAMM)

install:
	@echo "Nothing to install yet. Fix it!!!"

