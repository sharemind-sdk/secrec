################################################################################
#
# CMake script for finding mpfr.
# The default CMake search process is used to locate files.
#
# This script creates the following variables:
#  MPFR_FOUND: Boolean that indicates if the package was found
#  MPFR_INCLUDE_DIRS: Paths to the necessary header files
#  MPFR_LIBRARIES: Package libraries
#
################################################################################

# Find headers and libraries
FIND_PATH(
    MPFR_INCLUDE_DIR
    NAMES
        gmp.h
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


# Set MPFR_FOUND honoring the QUIET and REQUIRED arguments
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    mpfr
    "Could NOT find MPFR"
    MPFR_LIBRARY MPFR_INCLUDE_DIR)

# Output variables
IF(MPFR_FOUND)
    SET(MPFR_INCLUDE_DIRS ${MPFR_INCLUDE_DIR})
    SET(MPFR_LIBRARIES ${MPFR_LIBRARY})
ENDIF(MPFR_FOUND)

# Advanced options for not cluttering the cmake UIs:
MARK_AS_ADVANCED(MPFR_INCLUDE_DIR MPFR_LIBRARY)
