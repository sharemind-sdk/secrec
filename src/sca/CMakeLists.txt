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
    "${CMAKE_SOURCE_DIR}/include/"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS})
ADD_EXECUTABLE("sca" "main.cpp")
SET_TARGET_PROPERTIES("sca" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.3.0.0")
TARGET_LINK_LIBRARIES("sca" "libscc"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${MPFR_LIBRARIES})
INSTALL(TARGETS "sca"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
