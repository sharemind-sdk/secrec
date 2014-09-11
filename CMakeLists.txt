#
# This file is a part of the Sharemind framework.
# Copyright (C) Cybernetica AS
#
# All rights are reserved. Reproduction in whole or part is prohibited
# without the written consent of the copyright owner. The usage of this
# code is subject to the appropriate license agreement.
#

CMAKE_MINIMUM_REQUIRED(VERSION "2.8.12")
PROJECT("SecreC framework" "CXX" "C")

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

ADD_COMPILE_OPTIONS(
  "-O2" "-Wall" "-Wextra"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-ggdb>"
  "$<$<NOT:$<STREQUAL:$<CONFIGURATION>,Release>>:-fno-omit-frame-pointer>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-DNDEBUG>"
  "$<$<STREQUAL:$<CONFIGURATION>,Release>:-fomit-frame-pointer>"
)
	
# We can't add c++11 flag with ADD_COMPILE_OPTIONS because it
# also adds the flag to the C compiler.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeIncludes/" ${CMAKE_MODULE_PATH})

find_package(Boost COMPONENTS thread system filesystem iostreams program_options REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(MPFR REQUIRED)
FIND_PACKAGE(SharemindLibas REQUIRED)

add_definitions("-DSHAREMIND_STDLIB_PATH=\"${CMAKE_INSTALL_PREFIX}/lib/sharemind/stdlib\"")

ADD_SUBDIRECTORY(src/libscc)
ADD_SUBDIRECTORY(src/sca)
ADD_SUBDIRECTORY(src/scc)
ADD_SUBDIRECTORY(tests)
