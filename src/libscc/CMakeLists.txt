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
    "${CMAKE_CURRENT_SOURCE_DIR}/ModuleInfo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/typechecker/Templates.h"
)
LIST(REMOVE_ITEM LIBSCC_HEADERS ${LIBSCC_HEADERS_P})

SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/treenode.cpp"
    PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

SET_SOURCE_FILES_PROPERTIES(
    "${LIBSCC_FLEX_GENERATED_HEADER}"
    "${LIBSCC_BISON_GENERATED_SOURCE}"
    "${LIBSCC_FLEX_SOURCE}"
    "${LIBSCC_BISON_SOURCE}"
    PROPERTIES
        COMPILE_OPTIONS "-Wno-c++-compat;-Wno-unused-macros;-Wno-switch-default"
        INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
    )
SharemindAddSharedLibrary(libscc
    OUTPUT_NAME "scc"
    SOURCES
        ${LIBSCC_SOURCES}
        ${LIBSCC_HEADERS}
        ${LIBSCC_HEADERS_P}
        "${LIBSCC_FLEX_GENERATED_HEADER}"
        "${LIBSCC_BISON_GENERATED_SOURCE}"
        "${LIBSCC_FLEX_SOURCE}"
        "${LIBSCC_BISON_SOURCE}"
    COMPONENT "libscc"
    SPLITDEBUG_COMPONENT "libscc-debug"
)
TARGET_INCLUDE_DIRECTORIES(libscc
    PUBLIC
        ${MPFR_INCLUDE_DIRS}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        # $<INSTALL_INTERFACE:include> # TODO
    )
TARGET_LINK_LIBRARIES(libscc
    PRIVATE
        Boost::filesystem
    PUBLIC
        Boost::boost
        ${MPFR_LIBRARIES}
        Sharemind::CHeaders
        Sharemind::CxxHeaders
    )


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
        "libc6 (>= 2.19)"
        "libmpfr${MPFR_ABI_VERSION}"
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
