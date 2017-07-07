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
    "VMFpu.h"
    "VMFpu.cpp"
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
    VERSION "${SECREC_FRAMEWORK_VERSION}")
TARGET_LINK_LIBRARIES("scc" "libscc"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${MPFR_LIBRARIES}
    ${SharemindLibas_LIBRARIES})
INSTALL(TARGETS "scc"
    COMPONENT "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")

# Packaging:
SET(CPACK_DEBIAN_SCC_PACKAGE_NAME "scc" PARENT_SCOPE)
SET(CPACK_COMPONENT_SCC_DESCRIPTION "Sharemind SecreC Compiler" PARENT_SCOPE)
SET(CPACK_DEBIAN_SCC_PACKAGE_SECTION "devel" PARENT_SCOPE)
SET(BV "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.0")
SET(deps
    "libboost-filesystem${BV}"
    "libboost-iostreams${BV}"
    "libboost-program-options${BV}"
    "libc6 (>= 2.19)"
    "libmpfr4"
    "libsharemind-as (>= 0.1.0)"
    "libsharemind-scc (= ${SECREC_FRAMEWORK_VERSION})"
    "libstdc++6 (>= 4.8.0)"
)
STRING(REPLACE ";" ", " deps "${deps}")
SET(CPACK_DEBIAN_SCC_PACKAGE_DEPENDS "${deps}" PARENT_SCOPE)
