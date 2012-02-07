#
# This file is a part of the Sharemind framework.
# Copyright (C) Cybernetica AS
#
# All rights are reserved. Reproduction in whole or part is prohibited
# without the written consent of the copyright owner. The usage of this
# code is subject to the appropriate license agreement.
#

CMAKE_MINIMUM_REQUIRED(VERSION "2.8")
CMAKE_POLICY(SET CMP0015 NEW)
PROJECT("SecreC framework" "CXX" "C")

LINK_DIRECTORIES("lib")
SET(COMMON_FLAGS "-DDEBUG -O0 -g3 -Wall -Wextra -Wstrict-aliasing -pedantic -pedantic-errors -Wno-long-long -pipe")
SET(CMAKE_C_FLAGS "--std=gnu99 ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS}")


################################################################################
# Load any custom configuration:
################################################################################

INCLUDE("config.local" OPTIONAL)

################################################################################
# libscc
################################################################################

find_package(Boost REQUIRED program_options)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(SECREC_PARSER "src/libscc/secrec.y"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")
FLEX_TARGET(SECREC_LEXER "src/libscc/secrec.l"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")
ADD_FLEX_BISON_DEPENDENCY(SECREC_LEXER SECREC_PARSER)

SET(FLEX_BISON_SOURCES
    "src/libscc/secrec.l"
    "src/libscc/secrec.y")

SET(LIBSCC_SOURCES
    "src/libscc/blocks.cpp"
    "src/libscc/icodelist.cpp"
    "src/libscc/imop.cpp"
    "src/libscc/intermediate.cpp"
    "src/libscc/misc.cpp"
    "src/libscc/dataflowanalysis.cpp"
    "src/libscc/symboltable.cpp"
    "src/libscc/treenode.cpp"
    "src/libscc/types.cpp"
    "src/libscc/virtual_machine.cpp"
    "src/libscc/constant.cpp"
    "src/libscc/symbol.cpp"
    "src/libscc/codegen/statement.cpp"
    "src/libscc/codegen/expression.cpp"
    "src/libscc/codegen/assign.cpp"
    "src/libscc/codegen/program.cpp"
    "src/libscc/codegen.cpp"
    "src/libscc/typechecker.cpp"
    "src/libscc/typechecker/templates.h"
    "src/libscc/typechecker/templates.cpp"
    "src/libscc/typechecker/procedures.cpp"
    "src/libscc/context_impl.h"
    "src/libscc/context_impl.cpp"
    "src/libscc/context.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

SET(LIBSCC_HEADERS
    "include/libscc/codegen.h"
    "include/libscc/dataflowanalysis.h"
    "include/libscc/imop.h"
    "include/libscc/log.h"
    "include/libscc/parser.h"
    "include/libscc/symbol.h"
    "include/libscc/treenode.h"
    "include/libscc/virtual_machine.h"
    "include/libscc/blocks.h"
    "include/libscc/codegenResult.h"
    "include/libscc/constant.h"
    "include/libscc/icodelist.h"
    "include/libscc/intermediate.h"
    "include/libscc/misc.h"
    "include/libscc/symboltable.h"
    "include/libscc/types.h"
    "include/libscc/typechecker.h"
    "include/libscc/context.h")

SET_SOURCE_FILES_PROPERTIES("src/libscc/treenode.cpp" PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

include_directories("include/libscc/" "src/libscc/")
ADD_LIBRARY("libscc" SHARED
    ${LIBSCC_SOURCES}
    ${FLEX_BISON_SOURCES}
    ${LIBSCC_HEADERS})
SET_TARGET_PROPERTIES("libscc" PROPERTIES
    OUTPUT_NAME "scc"
    LIBRARY_OUTPUT_DIRECTORY "lib/"
    VERSION "0.3.0.0"
    SOVERSION "0.3")

################################################################################
# sca
################################################################################

include_directories("include/")
ADD_EXECUTABLE("sca" "src/sca/main.cpp")
SET_TARGET_PROPERTIES("sca" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin/"
    VERSION "0.3.0.0")
TARGET_LINK_LIBRARIES("sca" "libscc")

################################################################################
# scc
################################################################################

include_directories("include/")
ADD_EXECUTABLE("scc"
    "src/scc/main.cpp"
    "src/scc/VMCode.h"
    "src/scc/VMCode.cpp"
    "src/scc/Compiler.h"
    "src/scc/Compiler.cpp"
    "src/scc/Builtin.h"
    "src/scc/Builtin.cpp"
    "src/scc/SyscallManager.h"
    "src/scc/SyscallManager.cpp"
    "src/scc/RegisterAllocator.h"
    "src/scc/RegisterAllocator.cpp"
    "src/scc/StringLiterals.h"
    "src/scc/StringLiterals.cpp"
    "src/scc/VMInstruction.h"
    "src/scc/VMInstruction.cpp"
    "src/scc/VMSymbolTable.h"
    "src/scc/VMSymbolTable.cpp"
    "src/scc/VMValue.h"
    "src/scc/VMValue.cpp"
    "src/scc/VMDataType.h")
SET_TARGET_PROPERTIES("scc" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "bin/"
    VERSION "0.0.1.0")
TARGET_LINK_LIBRARIES("scc" "libscc" ${Boost_PROGRAM_OPTIONS_LIBRARY})

################################################################################
# Installs:
################################################################################

INSTALL(TARGETS "libscc" "sca" "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")

################################################################################
# Tests:
################################################################################

ENABLE_TESTING()

FUNCTION(add_test_secrec_execute testfile)
    ADD_TEST(NAME "${testfile}"
        COMMAND $<TARGET_FILE:sca> --eval "${CMAKE_CURRENT_SOURCE_DIR}/tests/regression/${testfile}.sc")
ENDFUNCTION()

# Tests for scalars:
add_test_secrec_execute("scalars/00-trivial")
add_test_secrec_execute("scalars/01-true")
add_test_secrec_execute("scalars/02-decl")
add_test_secrec_execute("scalars/03-scope")
add_test_secrec_execute("scalars/04-function")
add_test_secrec_execute("scalars/05-assert-fail")
SET_TESTS_PROPERTIES("scalars/05-assert-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "assert failed at \\(2,13\\)\\(4,2\\)")
add_test_secrec_execute("scalars/05-declassify")
add_test_secrec_execute("scalars/06-rec")
add_test_secrec_execute("scalars/07-while")
add_test_secrec_execute("scalars/08-func-ret")
add_test_secrec_execute("scalars/09-global")
add_test_secrec_execute("scalars/10-if")
add_test_secrec_execute("scalars/11-empty-if")
add_test_secrec_execute("scalars/12-return")
add_test_secrec_execute("scalars/13-loop-bench")
add_test_secrec_execute("scalars/14-expressions")
add_test_secrec_execute("scalars/15-continue")
add_test_secrec_execute("scalars/16-break")
add_test_secrec_execute("scalars/17-for")
add_test_secrec_execute("scalars/18-for-empty-body")
add_test_secrec_execute("scalars/19-logic-public")
add_test_secrec_execute("scalars/20-logic-bug1")
add_test_secrec_execute("scalars/21-logic-private")
add_test_secrec_execute("scalars/22-ternary")
add_test_secrec_execute("scalars/23-uops-public")
add_test_secrec_execute("scalars/24-uops-private")
add_test_secrec_execute("scalars/25-assignops")
add_test_secrec_execute("scalars/26-do-while")
add_test_secrec_execute("scalars/27-do-while-bug")
add_test_secrec_execute("scalars/28-scope-bug")
add_test_secrec_execute("scalars/29-scope-bug-2")
add_test_secrec_execute("scalars/30-rec-bug")
add_test_secrec_execute("scalars/31-fib")
add_test_secrec_execute("scalars/32-overloading")
add_test_secrec_execute("scalars/33-string")
add_test_secrec_execute("scalars/34-print")
add_test_secrec_execute("scalars/35-prefix-incdec")
add_test_secrec_execute("scalars/36-postfix-incdec")
add_test_secrec_execute("scalars/37-for-decl")
add_test_secrec_execute("scalars/38-postfix-fail")
add_test_secrec_execute("scalars/39-int-types")
add_test_secrec_execute("scalars/40-casts")
add_test_secrec_execute("scalars/41-param-order")
add_test_secrec_execute("scalars/42-ra-fail")
add_test_secrec_execute("scalars/43-domain-fail")
SET_TESTS_PROPERTIES("scalars/43-domain-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(9,5\\)\\(9,12\\)")
add_test_secrec_execute("scalars/44-classify")
add_test_secrec_execute("scalars/45-return-type-overloading")
add_test_secrec_execute("scalars/46-domain-id")
add_test_secrec_execute("scalars/47-operator-overloading")
add_test_secrec_execute("scalars/48-multiple-var-decl")
add_test_secrec_execute("scalars/49-multiple-var-decl-bug")
add_test_secrec_execute("scalars/50-integer-types")
add_test_secrec_execute("scalars/51-qualified-expressions")

# Tests for arrays:
add_test_secrec_execute("arrays/00-trivia")
add_test_secrec_execute("arrays/01-size")
add_test_secrec_execute("arrays/02-expression")
add_test_secrec_execute("arrays/03-index")
add_test_secrec_execute("arrays/04-slice")
add_test_secrec_execute("arrays/05-simplicit")
add_test_secrec_execute("arrays/06-functions")
add_test_secrec_execute("arrays/07-dynamic-size")
add_test_secrec_execute("arrays/08-rec")
add_test_secrec_execute("arrays/09-reshape")
add_test_secrec_execute("arrays/10-shape")
add_test_secrec_execute("arrays/11-shape-change")
add_test_secrec_execute("arrays/12-big-expr")
add_test_secrec_execute("arrays/13-index-assing")
add_test_secrec_execute("arrays/14-index-assing-slice")
add_test_secrec_execute("arrays/15-index-assing-slice-2d")
add_test_secrec_execute("arrays/16-scalar-reshape")
add_test_secrec_execute("arrays/17-assign-op-index")
add_test_secrec_execute("arrays/18-nat")
add_test_secrec_execute("arrays/19-fibo")
add_test_secrec_execute("arrays/20-order")
add_test_secrec_execute("arrays/21-reshape-bug")
add_test_secrec_execute("arrays/22-ternary-bug")
add_test_secrec_execute("arrays/23-assign-bug")
add_test_secrec_execute("arrays/24-ternary")
add_test_secrec_execute("arrays/25-uops")
add_test_secrec_execute("arrays/26-classify")
add_test_secrec_execute("arrays/27-assign-op")
add_test_secrec_execute("arrays/28-primes")
add_test_secrec_execute("arrays/29-slice-suggar")
add_test_secrec_execute("arrays/30-concat")
add_test_secrec_execute("arrays/31-concat-2d")
add_test_secrec_execute("arrays/32-slice-order")
add_test_secrec_execute("arrays/33-concat-suggar")
add_test_secrec_execute("arrays/34-reshape-classify")
add_test_secrec_execute("arrays/35-cat-bug")
add_test_secrec_execute("arrays/36-slice-empty-dim")
add_test_secrec_execute("arrays/37-cat-empty")
add_test_secrec_execute("arrays/38-concat-empty-matrix-bug")
add_test_secrec_execute("arrays/39-size-of-assignment-expr")
add_test_secrec_execute("arrays/40-prefix-postfix-incdec")
add_test_secrec_execute("arrays/41-for-decl")
add_test_secrec_execute("arrays/42-inc-bug")
add_test_secrec_execute("arrays/43-arith-assignment-bug")
add_test_secrec_execute("arrays/44-casts")
add_test_secrec_execute("arrays/45-inc-bug-2")

# Tests for templates:
add_test_secrec_execute("templates/00-trivial")
add_test_secrec_execute("templates/01-recursion")
add_test_secrec_execute("templates/02-kind-fail")
SET_TESTS_PROPERTIES("templates/02-kind-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(8,5\\)\\(8,19\\).")
add_test_secrec_execute("templates/03-kind")
add_test_secrec_execute("templates/04-multiparam")
add_test_secrec_execute("templates/05-multiparam-fail")
SET_TESTS_PROPERTIES("templates/05-multiparam-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(12,5\\)\\(12,15\\).")
add_test_secrec_execute("templates/06-instantiation-fail")
SET_TESTS_PROPERTIES("templates/06-instantiation-fail"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(4,9\\)\\(4,10\\)")
add_test_secrec_execute("templates/07-strange-bug")
add_test_secrec_execute("templates/08-mutual-recursion")
add_test_secrec_execute("templates/09-recursive-call-of-different-instance")
add_test_secrec_execute("templates/10-overloading")
add_test_secrec_execute("templates/11-name-lookup")
add_test_secrec_execute("templates/12-return-type-overloading")
add_test_secrec_execute("templates/13-various")
add_test_secrec_execute("templates/14-ambiguous-lookup")
SET_TESTS_PROPERTIES("templates/14-ambiguous-lookup"
    PROPERTIES PASS_REGULAR_EXPRESSION "[FATAL].*\\(14,5\\)\\(14,15\\)")


# Run Qt4 tests if qt is found
find_package(Qt4)
IF (QT4_FOUND)

    SET(TEST_LIBRARIES "libscc" ${QT_LIBRARIES} ${QT_QTTEST_LIBRARIES})
    foreach(testname "testparse" "testtreenode")
        include_directories("include"
            "tests/libscc/${testname}/"
            ${QT_INCLUDE_DIR}
            ${QT_QTTEST_INCLUDE_DIR})
        SET(QT4_TEST_MOC)
        QT4_WRAP_CPP(QT4_TEST_MOC "tests/libscc/${testname}/${testname}.h")
        ADD_EXECUTABLE("test-libscc-${testname}"
            "tests/libscc/${testname}/${testname}.cpp"
            ${QT4_TEST_MOC})
        SET_TARGET_PROPERTIES("test-libscc-${testname}" PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "bin/")
        TARGET_LINK_LIBRARIES("test-libscc-${testname}" ${TEST_LIBRARIES})
    endforeach()


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

    SET(QT4_TREENODE_TESTS
        "testInitTreeNode"
        "testInitBool"
        "testInitInt"
        "testInitUInt"
        "testInitString"
        "testInitType"
        "testChildren")
    foreach(casename ${QT4_TREENODE_TESTS})
        ADD_TEST(NAME ${casename}
            COMMAND $<TARGET_FILE:test-libscc-testtreenode> ${casename})
    endforeach()
ENDIF()
