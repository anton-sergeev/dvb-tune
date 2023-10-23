cmake_minimum_required(VERSION 2.8)

set(PROJECT dvb-tune)
project(${PROJECT} C)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  # add_definitions("-g")
  add_definitions("-Wall")
  add_definitions("-Wextra")
  # add_definitions("-Wpedantic")
  #add_definitions("-Werror")
endif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")

set(SOURCES
  tune.c
)

add_executable(${PROJECT} ${SOURCES})

