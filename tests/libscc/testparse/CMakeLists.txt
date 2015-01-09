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
# Some Qt parser tests:
################################################################################

SET(TEST_NAME "testparse")
include_directories("${CMAKE_SOURCE_DIR}/include"
    ${QT_INCLUDE_DIR}
    ${QT_QTTEST_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS})
SET(QT4_TEST_MOC)
QT4_WRAP_CPP(QT4_TEST_MOC "${TEST_NAME}.h")
ADD_EXECUTABLE("test-libscc-${TEST_NAME}"
    "${TEST_NAME}.cpp"
    ${QT4_TEST_MOC})
SET_TARGET_PROPERTIES("test-libscc-${TEST_NAME}" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
TARGET_LINK_LIBRARIES("test-libscc-${TEST_NAME}"
    "libscc" ${QT_QTCORE_LIBRARIES} ${QT_QTTEST_LIBRARIES} ${MPFR_LIBRARY})

SET(QT4_PARSER_TESTS
    "testParseProgram"
    "testGlobalDecls"
    "testStmtCompound"
    "testStmtIf"
    "testStmtFor"
    "testStmtWhile"
    "testStmtDoWhile"
    "testStmtOther"
    "testExprPrimary"
    "testExprPostfix"
    "testExprUnary"
    "testExprCast"
    "testExprMult"
    "testExprAddi"
    "testExprRela"
    "testExprEqua"
    "testExprLAnd"
    "testExprLOr"
    "testExprCond"
    "testExprAssign"
    "testExprPrecedence"
    "testInlineDecls")
foreach(casename ${QT4_PARSER_TESTS})
    ADD_TEST(NAME ${casename}
        COMMAND $<TARGET_FILE:test-libscc-testparse> ${casename})
endforeach()
