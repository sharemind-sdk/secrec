################################################################################
# Some Qt parser tests:
################################################################################

SET(TEST_NAME "testparse")
include_directories("${CMAKE_SOURCE_DIR}/include"
    ${QT_INCLUDE_DIR}
    ${QT_QTTEST_INCLUDE_DIR})
SET(QT4_TEST_MOC)
QT4_WRAP_CPP(QT4_TEST_MOC "${TEST_NAME}.h")
ADD_EXECUTABLE("test-libscc-${TEST_NAME}"
    "${TEST_NAME}.cpp"
    ${QT4_TEST_MOC})
SET_TARGET_PROPERTIES("test-libscc-${TEST_NAME}" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
TARGET_LINK_LIBRARIES("test-libscc-${TEST_NAME}"
    "libscc" ${QT_QTCORE_LIBRARIES} ${QT_QTTEST_LIBRARIES})

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
    "testExprMatrix"
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
