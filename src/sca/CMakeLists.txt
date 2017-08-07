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
# sca
################################################################################

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(
    "${CMAKE_SOURCE_DIR}/src/"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS})

FILE(GLOB_RECURSE SCA_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

ADD_EXECUTABLE("sca" ${SCA_SOURCES})
SET_TARGET_PROPERTIES("sca" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "${SECREC_FRAMEWORK_VERSION}")
TARGET_LINK_LIBRARIES("sca" "libscc"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${MPFR_LIBRARIES})
INSTALL(TARGETS "sca"
    COMPONENT "sca"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")

# Packaging:
SET(CPACK_DEBIAN_SCA_PACKAGE_NAME "sca" PARENT_SCOPE)
SET(CPACK_COMPONENT_SCA_DESCRIPTION "Sharemind SecreC Analyzer" PARENT_SCOPE)
SET(CPACK_DEBIAN_SCA_PACKAGE_SECTION "devel" PARENT_SCOPE)
SET(BV "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.0")
SET(deps
    "libboost-iostreams${BV}"
    "libboost-program-options${BV}"
    "libc6 (>= 2.19)"
    "libsharemind-scc (= ${SECREC_FRAMEWORK_VERSION})"
    "libstdc++6 (>= 4.8.0)"
)
STRING(REPLACE ";" ", " deps "${deps}")
SET(CPACK_DEBIAN_SCA_PACKAGE_DEPENDS "${deps}" PARENT_SCOPE)
