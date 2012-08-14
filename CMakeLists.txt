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

INCLUDE("config.local" OPTIONAL)

MACRO(SHAREMIND_ADD_EXT name pname)
    IF(NOT SHAREMIND_NO_BUILD_EXTS)
        SET(ID "${CMAKE_CURRENT_SOURCE_DIR}/ext/${name}/src")
        IF(NOT EXISTS "${ID}/sharemind/CMakeLists.txt")
            MESSAGE(FATAL_ERROR "Sharemind extension ${name} not found!")
        ELSE()
            ADD_SUBDIRECTORY("${ID}/sharemind")
            INCLUDE_DIRECTORIES(BEFORE SYSTEM
                                "${${pname}_SOURCE_DIR}/.."
                                "${${pname}_BINARY_DIR}/..")
        ENDIF()
    ENDIF(NOT SHAREMIND_NO_BUILD_EXTS)
ENDMACRO(SHAREMIND_ADD_EXT)

INCLUDE_DIRECTORIES(BEFORE SYSTEM
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

SHAREMIND_ADD_EXT(vm_globals SHAREMIND_VM_GLOBALS)
SHAREMIND_ADD_EXT(vm_m4 SHAREMIND_VM_M4)
SHAREMIND_ADD_EXT(libexecutable SHAREMIND_LIBEXECUTABLE)
SHAREMIND_ADD_EXT(libvmi SHAREMIND_LIBVMI)
SHAREMIND_ADD_EXT(libas SHAREMIND_LIBAS)

find_package(Boost COMPONENTS thread system filesystem iostreams program_options REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Qt4 COMPONENTS qtcore qttest)

ADD_SUBDIRECTORY(src/libscc)
ADD_SUBDIRECTORY(src/sca)
ADD_SUBDIRECTORY(src/scc)
ADD_SUBDIRECTORY(tests)
