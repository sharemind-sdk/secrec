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


FILE(GLOB_RECURSE SCC_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
SharemindAddExecutable(scc
    SOURCES ${SCC_SOURCES}
    COMPONENT "scc"
    SPLITDEBUG_COMPONENT "scc-debug"
)
TARGET_INCLUDE_DIRECTORIES(scc
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src/"
        ${MPFR_INCLUDE_DIRS}
    )
TARGET_LINK_LIBRARIES(scc
    PRIVATE
        Boost::boost
        Boost::filesystem
        Boost::iostreams
        Boost::program_options
        libscc
        ${MPFR_LIBRARIES}
        Sharemind::CxxHeaders
        Sharemind::LibAs
        Sharemind::LibExecutable
    )


# Packaging:
SET(BV
    "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}"
)

SharemindAddComponentPackage("scc"
    NAME "scc"
    DESCRIPTION "Sharemind SecreC Compiler"
    DEB_SECTION "devel"
    DEB_DEPENDS
        "libboost-filesystem${BV}"
        "libboost-iostreams${BV}"
        "libboost-program-options${BV}"
        "libc6 (>= 2.19)"
        "libmpfr${MPFR_ABI_VERSION}"
        "libsharemind-as (>= 0.4.0)"
        "libsharemind-executable (>= 0.4.0)"
        "libsharemind-scc (= ${SharemindSecrecFramework_DEB_libscc_PACKAGE_VERSION})"
        "libstdc++6 (>= 4.8.0)"
    PARENT_SCOPE
)
SharemindAddComponentPackage("scc-debug"
    NAME "scc-dbg"
    DESCRIPTION "Sharemind SecreC Compiler debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS
        "scc (= ${SharemindSecrecFramework_DEB_scc_PACKAGE_VERSION})"
    PARENT_SCOPE
)
