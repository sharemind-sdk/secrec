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

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(SharemindSecrecFramework VERSION 2.2.0 LANGUAGES C CXX)

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/config.local" OPTIONAL)
INCLUDE("${CMAKE_CURRENT_BINARY_DIR}/config.local" OPTIONAL)

FIND_PACKAGE(SharemindCMakeHelpers 1.2 REQUIRED)

SharemindSetProjectVersion()


SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeIncludes/" ${CMAKE_MODULE_PATH})

find_package(Boost COMPONENTS filesystem iostreams program_options system REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(MPFR REQUIRED)
FIND_PACKAGE(SharemindCHeaders 1.1.0 REQUIRED)
FIND_PACKAGE(SharemindCxxHeaders 0.2.0 REQUIRED)
FIND_PACKAGE(SharemindLibAs 0.3.0 REQUIRED)
FIND_PACKAGE(SharemindLibExecutable 0.2.0 REQUIRED)

add_definitions("-DSHAREMIND_STDLIB_PATH=\"${CMAKE_INSTALL_PREFIX}/lib/sharemind/stdlib\"")

SharemindSetupPackaging()
ADD_SUBDIRECTORY(src/libscc)
ADD_SUBDIRECTORY(src/sca)
ADD_SUBDIRECTORY(src/scc)
ADD_SUBDIRECTORY(tests)
SharemindPackagingFinalize()
