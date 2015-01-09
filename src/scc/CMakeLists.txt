#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

################################################################################
# scc
################################################################################

INCLUDE_DIRECTORIES(${SharemindLibas_INCLUDE_DIRS})
ADD_DEFINITIONS(${SharemindLibas_DEFINITIONS})

SET(SCC_SOURCES
    "main.cpp"
    "VMCode.h"
    "VMCode.cpp"
    "Compiler.h"
    "Compiler.cpp"
    "Builtin.h"
    "Builtin.cpp"
    "SyscallManager.h"
    "SyscallManager.cpp"
    "RegisterAllocator.h"
    "RegisterAllocator.cpp"
    "StringLiterals.h"
    "StringLiterals.cpp"
    "VMInstruction.h"
    "VMInstruction.cpp"
    "VMSymbolTable.h"
    "VMSymbolTable.cpp"
    "VMValue.h"
    "VMValue.cpp"
    "VMDataType.h")

include_directories(
    "${CMAKE_SOURCE_DIR}/include/"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS})
ADD_EXECUTABLE("scc" ${SCC_SOURCES})
SET_TARGET_PROPERTIES("scc" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.0.1.0")
TARGET_LINK_LIBRARIES("scc" "libscc"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${MPFR_LIBRARIES}
    ${SharemindLibas_LIBRARIES})
INSTALL(TARGETS "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
