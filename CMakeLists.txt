#
# This file is a part of the Sharemind framework.
# Copyright (C) Cybernetica AS
#
# All rights are reserved. Reproduction in whole or part is prohibited
# without the written consent of the copyright owner. The usage of this
# code is subject to the appropriate license agreement.
#

CMAKE_MINIMUM_REQUIRED(VERSION "2.8")
CMAKE_POLICY(SET CMP0015 NEW)
PROJECT("SecreC framework" "CXX" "C")

LINK_DIRECTORIES("lib")
SET(COMMON_FLAGS "-DDEBUG -O0 -g3 -Wall -Wextra -Wstrict-aliasing -pedantic -pedantic-errors -Wno-long-long -pipe")
SET(CMAKE_C_FLAGS "--std=gnu99 ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS}")

INCLUDE("config.local" OPTIONAL)
add_subdirectory (src)

# \todo check for some flag
ENABLE_TESTING ()
add_subdirectory (tests)
