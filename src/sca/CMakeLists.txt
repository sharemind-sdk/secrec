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

FILE(GLOB_RECURSE SCA_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

SharemindAddExecutable("sca"
    SOURCES
        ${SCA_SOURCES}
    INCLUDE_DIRECTORIES
        "${CMAKE_SOURCE_DIR}/src/"
        ${Boost_INCLUDE_DIRS}
        ${MPFR_INCLUDE_DIRS}
    LINK_LIBRARIES
        "libscc"
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        ${MPFR_LIBRARIES}
    COMPONENT "sca"
    SPLITDEBUG_COMPONENT "sca-debug"
)
SharemindSetCxx11CompileOptions(TARGETS "sca")


# Packaging:
SET(BV "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.0")
SharemindAddComponentPackage("sca"
    NAME "sca"
    DESCRIPTION "Sharemind SecreC Analyzer"
    DEB_SECTION "devel"
    DEB_DEPENDS
        "libboost-iostreams${BV}"
        "libboost-program-options${BV}"
        "libc6 (>= 2.19)"
        "libsharemind-scc (= ${SharemindSecrecFramework_VERSION})"
        "libstdc++6 (>= 4.8.0)"
    PARENT_SCOPE
)
SharemindAddComponentPackage("sca-debug"
    NAME "sca-dbg"
    DESCRIPTION "Sharemind SecreC Analyzer debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS
        "sca (= ${SharemindSecrecFramework_VERSION})"
    PARENT_SCOPE
)
