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


FILE(GLOB_RECURSE SCA_SOURCES
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
SharemindAddExecutable(sca
    SOURCES ${SCA_SOURCES}
    COMPONENT "sca"
    SPLITDEBUG_COMPONENT "sca-debug"
)
TARGET_INCLUDE_DIRECTORIES(sca
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src/"
        ${MPFR_INCLUDE_DIRS}
    )
TARGET_LINK_LIBRARIES(sca
    PRIVATE
        Boost::boost
        Boost::program_options
        Boost::iostreams
        libscc
        ${MPFR_LIBRARIES}
        Sharemind::CxxHeaders
    )


# Packaging:
SET(BV
    "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}"
)

SharemindAddComponentPackage("sca"
    NAME "sca"
    DESCRIPTION "Sharemind SecreC Analyzer"
    DEB_SECTION "devel"
    DEB_DEPENDS
        "libboost-iostreams${BV}"
        "libboost-program-options${BV}"
        "libc6 (>= 2.19)"
        "libsharemind-scc (= ${SharemindSecrecFramework_DEB_libscc_PACKAGE_VERSION})"
        "libstdc++6 (>= 4.8.0)"
    PARENT_SCOPE
)
SharemindAddComponentPackage("sca-debug"
    NAME "sca-dbg"
    DESCRIPTION "Sharemind SecreC Analyzer debug symbols"
    DEB_SECTION "debug"
    DEB_DEPENDS
        "sca (= ${SharemindSecrecFramework_DEB_sca_PACKAGE_VERSION})"
    PARENT_SCOPE
)
