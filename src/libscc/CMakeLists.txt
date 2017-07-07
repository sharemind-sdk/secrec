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

FIND_PACKAGE(SharemindCHeaders REQUIRED)
ADD_DEFINITIONS(${SharemindCHeaders_DEFINITIONS})

################################################################################
# libscc
################################################################################

SET(LIBSCC_FLEX_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/secrec.l")
SET(LIBSCC_FLEX_GENERATED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")
FLEX_TARGET(SECREC_LEXER "${LIBSCC_FLEX_SOURCE}"
            "${LIBSCC_FLEX_GENERATED_HEADER}")

SET(LIBSCC_BISON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/secrec.y")
SET(LIBSCC_BISON_GENERATED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")
BISON_TARGET(SECREC_PARSER "${LIBSCC_BISON_SOURCE}"
             "${LIBSCC_BISON_GENERATED_SOURCE}")

ADD_FLEX_BISON_DEPENDENCY(SECREC_LEXER SECREC_PARSER)

SET(LIBSCC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/libscc/")

FILE(GLOB_RECURSE LIBSCC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB_RECURSE LIBSCC_HEADERS "${LIBSCC_INCLUDE_DIR}/*.h")
FILE(GLOB_RECURSE LIBSCC_HEADERS_P "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/treenode.cpp"
    PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

include_directories(
    "${LIBSCC_INCLUDE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS}
    ${SharemindCHeaders_INCLUDE_DIRS})

IF(NOT SHAREMIND_LIBSCC_LINKING_MODE)
    SET(SHAREMIND_LIBSCC_LINKING_MODE SHARED)
ENDIF()

ADD_LIBRARY("libscc" ${SHAREMIND_LIBSCC_LINKING_MODE}
    ${LIBSCC_SOURCES}
    ${LIBSCC_HEADERS}
    ${LIBSCC_HEADERS_P}
    "${LIBSCC_FLEX_GENERATED_HEADER}"
    "${LIBSCC_BISON_GENERATED_SOURCE}"
    "${LIBSCC_FLEX_SOURCE}"
    "${LIBSCC_BISON_SOURCE}"
)
SET_TARGET_PROPERTIES("libscc" PROPERTIES
    OUTPUT_NAME "scc"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    VERSION "${SECREC_FRAMEWORK_VERSION}"
    SOVERSION "${SECREC_FRAMEWORK_SOVERSION}"
    CLEAN_DIRECT_OUTPUT 1)
TARGET_LINK_LIBRARIES("libscc"
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${MPFR_LIBRARIES}
    ${SharemindCHeaders_LIBRARIES})
INSTALL(TARGETS "libscc"
    COMPONENT "lib"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")


# Packaging:
SET(CPACK_DEBIAN_LIB_PACKAGE_NAME "libsharemind-scc" PARENT_SCOPE)
SET(CPACK_COMPONENT_LIB_DESCRIPTION "Sharemind SecreC Compiler library"
    PARENT_SCOPE)
SET(CPACK_DEBIAN_LIB_PACKAGE_SECTION "libs" PARENT_SCOPE)
SET(BV "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.0")
SET(deps
    "libboost-filesystem${BV}"
    "libc6 (>= 2.19)"
    "libmpfr4"
    "libstdc++6 (>= 4.8.0)"
)
STRING(REPLACE ";" ", " deps "${deps}")
SET(CPACK_DEBIAN_LIB_PACKAGE_DEPENDS "${deps}" PARENT_SCOPE)
