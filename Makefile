
BUILD_DIR ?= cmake_build/x86_64

all: build


build: "$(BUILD_DIR)"
	cmake --build "$(BUILD_DIR)"

"$(BUILD_DIR)" :
	cmake -S . -B "$(BUILD_DIR)"

