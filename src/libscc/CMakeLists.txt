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


SET(LIBSCC_FLEX_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/secrec.l")
SET(LIBSCC_FLEX_GENERATED_HEADER "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")
FLEX_TARGET(SECREC_LEXER "${LIBSCC_FLEX_SOURCE}"
            "${LIBSCC_FLEX_GENERATED_HEADER}")

SET(LIBSCC_BISON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/secrec.y")
SET(LIBSCC_BISON_GENERATED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")
BISON_TARGET(SECREC_PARSER "${LIBSCC_BISON_SOURCE}"
             "${LIBSCC_BISON_GENERATED_SOURCE}")

ADD_FLEX_BISON_DEPENDENCY(SECREC_LEXER SECREC_PARSER)

FILE(GLOB_RECURSE LIBSCC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB_RECURSE LIBSCC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB_RECURSE LIBSCC_HEADERS_P
    "${CMAKE_CURRENT_SOURCE_DIR}/ContextImpl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ModuleInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/typechecker/Templates.h"
)
LIST(REMOVE_ITEM LIBSCC_HEADERS ${LIBSCC_HEADERS_P})

SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/treenode.cpp"
    PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

ADD_LIBRARY("yacc_o" STATIC
    "${LIBSCC_FLEX_GENERATED_HEADER}"
    "${LIBSCC_BISON_GENERATED_SOURCE}"
    "${LIBSCC_FLEX_SOURCE}"
    "${LIBSCC_BISON_SOURCE}"
)
SharemindSetC99CompileOptions(
    TARGETS "yacc_o"
    FORCED_OPTIONS "-fPIC"
    CHECK_OPTIONS
        "-Wno-c++-compat"
        "-Wno-unused-macros"
        "-Wno-switch-default"
)
SET_PROPERTY(TARGET "yacc_o"
    PROPERTY INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}"
)


SharemindAddSharedLibrary("libscc"
    OUTPUT_NAME "scc"
    SOURCES
        ${LIBSCC_SOURCES}
        ${LIBSCC_HEADERS}
        ${LIBSCC_HEADERS_P}
    INCLUDE_DIRECTORIES
        ${Boost_INCLUDE_DIRS}
        ${MPFR_INCLUDE_DIRS}
        ${SharemindCHeaders_INCLUDE_DIRS}
        ${SharemindCxxHeaders_INCLUDE_DIRS}
    LEGACY_DEFINITIONS
        ${SharemindCHeaders_DEFINITIONS}
        ${SharemindCxxHeaders_DEFINITIONS}
    LINK_LIBRARIES
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${MPFR_LIBRARIES}
        ${SharemindCHeaders_LIBRARIES}
        ${SharemindCxxHeaders_LIBRARIES}
        "yacc_o"
    COMPONENT "libscc"
    SPLITDEBUG_COMPONENT "libscc-debug"
)
SharemindSetCxx11CompileOptions(TARGETS "libscc")


# Packaging:
SET(BV
    "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}"
)

SharemindAddComponentPackage("libscc"
    NAME "libsharemind-scc"
    DESCRIPTION "Sharemind SecreC Compiler library"
    DEB_SECTION "libs"
    DEB_DEPENDS
        "libboost-filesystem${BV}"
        "libboost-system${BV}"
        "libc6 (>= 2.19)"
        "libmpfr4 | libmpfr6"
        "libstdc++6 (>= 4.8.0)"
    PARENT_SCOPE
)
SharemindAddComponentPackage("libscc-debug"
    NAME "libsharemind-scc-dbg"
    DESCRIPTION "Sharemind SecreC Compiler library debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS
        "libsharemind-scc (= ${SharemindSecrecFramework_DEB_libscc_PACKAGE_VERSION})"
    PARENT_SCOPE
)

# Propagate package version to script in parent directory:
SET(SharemindSecrecFramework_DEB_libscc_PACKAGE_VERSION
    "${SharemindSecrecFramework_DEB_libscc_PACKAGE_VERSION}" PARENT_SCOPE)
