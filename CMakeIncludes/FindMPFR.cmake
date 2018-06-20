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
#
# CMake script for finding mpfr.
# The default CMake search process is used to locate files.
#
# This script creates the following variables:
#  MPFR_FOUND: Boolean that indicates if the package was found
#  MPFR_INCLUDE_DIRS: Paths to the necessary header files
#  MPFR_LIBRARIES: Package libraries
#  MPFR_VERSION: MPFR version
#  MPFR_ABI_VERSION: MPFR ABI version (either 4 or 6)
#
################################################################################

# Find headers and libraries
FIND_PATH(
    MPFR_INCLUDE_DIR
    NAMES
        mpfr.h
    HINTS
        $ENV{MPFR_ROOT}
        ${MPFR_ROOT}
    PATHS
        /usr/local
        /usr
        /opt/local
    PATH_SUFFIXES
        include
)

FIND_LIBRARY(
    MPFR_LIBRARY
    NAMES
        mpfr
    HINTS
        $ENV{MPFR_ROOT}
        ${MPFR_ROOT}
    PATHS
        /usr/local
        /usr
        /opt/local
    PATH_SUFFIXES
        lib
)

IF(MPFR_INCLUDE_DIR)
    # Query MPFR_VERSION
    FILE(READ "${MPFR_INCLUDE_DIR}/mpfr.h" _mpfr_header)

    STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_MAJOR[ \t]+([0-9]+)"
        _mpfr_major_version_match "${_mpfr_header}")
    SET(MPFR_MAJOR_VERSION "${CMAKE_MATCH_1}")
    STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_MINOR[ \t]+([0-9]+)"
        _mpfr_minor_version_match "${_mpfr_header}")
    SET(MPFR_MINOR_VERSION "${CMAKE_MATCH_1}")
    STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_PATCHLEVEL[ \t]+([0-9]+)"
        _mpfr_patchlevel_version_match "${_mpfr_header}")
    SET(MPFR_PATCHLEVEL_VERSION "${CMAKE_MATCH_1}")

    SET(MPFR_VERSION
        ${MPFR_MAJOR_VERSION}.${MPFR_MINOR_VERSION}.${MPFR_PATCHLEVEL_VERSION})

    IF(${MPFR_VERSION} VERSION_LESS "3.0.0")
        SET(MPFR_VERSION_OK FALSE)
        MESSAGE(STATUS "MPFR version ${MPFR_VERSION} found in ${MPFR_INCLUDES}, "
            "but at least version 3.0.0 is required")
    ELSEIF(${MPFR_VERSION} VERSION_LESS "4.0.0")
        SET(MPFR_VERSION_OK TRUE)
        SET(MPFR_ABI_VERSION "4")
    ELSE()
        SET(MPFR_VERSION_OK TRUE)
        SET(MPFR_ABI_VERSION "6")
    ENDIF()

ENDIF()

# Set MPFR_FOUND honoring the QUIET and REQUIRED arguments
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    mpfr
    "Could NOT find MPFR"
    MPFR_LIBRARY MPFR_INCLUDE_DIR MPFR_VERSION_OK)

# Output variables
IF(MPFR_FOUND)
    SET(MPFR_INCLUDE_DIRS ${MPFR_INCLUDE_DIR})
    SET(MPFR_LIBRARIES ${MPFR_LIBRARY})
ENDIF(MPFR_FOUND)

# Advanced options for not cluttering the cmake UIs:
MARK_AS_ADVANCED(MPFR_INCLUDE_DIR MPFR_LIBRARY)
