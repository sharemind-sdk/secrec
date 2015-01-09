/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include "testparse.h"

#include <libscc/parser.h>
#include <libscc/treenode.h>
#include <libscc/StringTable.h>
#include <string>
#include <sstream>

using namespace SecreC;

#define ADD_STANDARD_COLUMNS \
    QTest::addColumn<QString>("sccInput");\
    QTest::addColumn<QString>("xmlOutput")

// XML elements with contents:
#define XB(a,b) "<" a ">" b "</" a ">"
#define XB2(a,b,c) XB(a, XB(b, c))
#define XB3(a,b,c,d) XB(a, XB2(b, c, d))

// Parser-specific XML elements:
#define XID(a) "<IDENTIFIER value=\"string:" a "\"/>"
#define XNONE  "<EXPR_NONE/>"
#define XRVAR(a) XB("EXPR_RVARIABLE", XID(a))
#define XTYPEVOID "<TYPEVOID/>"
#define NODIMENSIONS "<DIMENSIONS/>"
#define XTYPETYPE(a,b,c) XB("TYPETYPE", a b c)
#define XTYPESECF(a) XB("SECTYPE_PRIVATE_F", XID(a))
#define XTYPESECF_PUBLIC "<SECTYPE_PUBLIC_F/>"
#define XTYPEDATAF(a) "<DATATYPE_CONST_F type=\"" a "\"/>"
#define XTYPEDIMF(a) "<DIMTYPE_CONST_F dim=\"" a "\"/>"
#define XTYPEDATAARRAY(a,b) "<DATATYPE_ARRAY dim=\"" #a "\">" b "</DATATYPE_ARRAY>"
#define XTYPEDATAARRAY2(a,b,c) XTYPEDATAARRAY(a,XTYPEDATAARRAY(b,c))
#define XSIMPLESTYPE(a,b,c) XTYPETYPE(XTYPESECF(a), XTYPEDATAF(b), XTYPEDIMF(c))
#define XSIMPLEPTYPE(a,b) XTYPETYPE(XTYPESECF_PUBLIC, XTYPEDATAF(a), XTYPEDIMF(b))
#define XDECL(a, b) XB("DECL", a XB("VAR_INIT", b))
#define XBOOL(a) "<LITE_BOOL value=\"bool:" #a "\"/>"
#define XINT(a) "<LITE_INT value=\"int:" #a "\"/>"
#define XSTR(a) "<LITE_STRING value=\"string:" a "\"/>"
#define XSCE "<STMT_COMPOUND/>"
#define XSTMTS(a) "<STMT_COMPOUND>" a "</STMT_COMPOUND>"

// Macros for testing simple code in blocks:
#define SIMPLE(x) "void myprocedure() {" x "}"
#define SIMPLE_PARSE(x) XB3("MODULE", "PROGRAM", "PROCDEF",\
        XID("myprocedure") XTYPEVOID x)
#define SIMPLE_PARSE_STMTS(x) SIMPLE_PARSE(XSTMTS(x))

// Macros for testing expressions as a statement:
#define SIMPLE_E(x) SIMPLE(x ";")
#define SIMPLE_PARSE_E(x) SIMPLE_PARSE_STMTS(XB("STMT_EXPR",x))

// Macros to test binary operators:
#define SIMPLE_BINARY_DETAIL(a,b,c) SIMPLE_E(a b c)
#define SIMPLE_BINARY(a) SIMPLE_BINARY_DETAIL("o1",a,"o2")
#define SIMPLE_PARSE_BINARY_DETAIL(a,b,c) SIMPLE_PARSE_E(XB("EXPR_BINARY_" b, XRVAR(a) XRVAR(c)))
#define SIMPLE_PARSE_BINARY(a) SIMPLE_PARSE_BINARY_DETAIL("o1",a,"o2")

// Macros to test the associativity and precedence of binary operators:
#define SIMPLE_ASSOC_DETAIL(a,b,c,d,e) SIMPLE_E(a b c d e)
#define SIMPLE_ASSOC(a,b) SIMPLE_ASSOC_DETAIL("o1",a,"o2",b,"o3")
#define SIMPLE_PARSE_ASSOC_LEFT_DETAIL(a,b,c,d,e) \
    SIMPLE_PARSE_E(XB("EXPR_BINARY_" d, XB("EXPR_BINARY_" b, XRVAR(a) XRVAR(c)) XRVAR(e)))
#define SIMPLE_PARSE_ASSOC_LEFT(a,b) SIMPLE_PARSE_ASSOC_LEFT_DETAIL("o1",a,"o2",b,"o3")
#define SIMPLE_PARSE_ASSOC_RIGHT_DETAIL(a,b,c,d,e) \
    SIMPLE_PARSE_E(XB("EXPR_BINARY_" b, XRVAR(a) XB("EXPR_BINARY_" d, XRVAR(c) XRVAR(e))))
#define SIMPLE_PARSE_ASSOC_RIGHT(a,b) SIMPLE_PARSE_ASSOC_RIGHT_DETAIL("o1",a,"o2",b,"o3")
#define ADD_ROWS_SINGLE_BINARY_LEFT(a,b,c) \
    QTest::newRow(a) << QString(SIMPLE_BINARY(b))\
                     << QString(SIMPLE_PARSE_BINARY(c));\
    QTest::newRow(a "Assoc") << QString(SIMPLE_ASSOC(b, b))\
                             << QString(SIMPLE_PARSE_ASSOC_LEFT(c, c));
#define ADD_ROWS_BINARY_RIGHT(a,b,c,d,e) \
    QTest::newRow(a) << QString(SIMPLE_ASSOC(b, d))\
                     << QString(SIMPLE_PARSE_ASSOC_RIGHT(c, e));\
    QTest::newRow(a "Inverse") << QString(SIMPLE_ASSOC(d, b))\
                               << QString(SIMPLE_PARSE_ASSOC_LEFT(e, c));

// Macros to test the associativity and precedence of binary assignment operators:
#define SIMPLE_PARSE_ASSIGN_DETAIL(a,b,c) SIMPLE_PARSE_E(XB("EXPR_BINARY_" b, XB("LVALUE", XID(a)) XRVAR(c)))
#define SIMPLE_PARSE_ASSIGN(a) SIMPLE_PARSE_ASSIGN_DETAIL("o1",a,"o2")
#define SIMPLE_PARSE_ASSIGN_ASSOC_RIGHT_DETAIL(a,b,c,d,e) \
    SIMPLE_PARSE_E(XB("EXPR_BINARY_" b, XB("LVALUE", XID(a)) XB("EXPR_BINARY_" d, XB("LVALUE", XID(c)) XRVAR(e))))
#define SIMPLE_PARSE_ASSIGN_ASSOC_RIGHT(a,b) \
    SIMPLE_PARSE_ASSIGN_ASSOC_RIGHT_DETAIL("o1",a,"o2",b,"o3")
#define ADD_ROWS_ASSIGN(a,b,c) \
    QTest::newRow(a) << QString(SIMPLE_BINARY(b))\
                     << QString(SIMPLE_PARSE_ASSIGN(c));\
    QTest::newRow(a "Assoc") << QString(SIMPLE_ASSOC(b, b))\
                             << QString(SIMPLE_PARSE_ASSIGN_ASSOC_RIGHT(c, c));


void TestParse::simpleParseTest() {
    QFETCH(QString, sccInput);
    QFETCH(QString, xmlOutput);

    TreeNodeModule *n;
    std::string input(sccInput.toStdString());
    std::ostringstream output;
    StringTable table;
    QCOMPARE(sccparse_mem(&table, "inMemory", input.c_str(), input.size(), &n), 0);
    n->printXml (output, false);
    QCOMPARE(QString(output.str ().c_str()), xmlOutput);
    delete n;
}

void TestParse::testParseProgram_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("oneprocedure")
        << QString("void myprocedure(){}")
        << QString(XB3("MODULE", "PROGRAM", "PROCDEF",
                       XID("myprocedure") XTYPEVOID XSCE));

    QTest::newRow("twoprocedures")
        << QString("void myprocedure(){}private int64[[1]] myprocedure2(){}")
        << QString(XB2("MODULE", "PROGRAM",
                       XB("PROCDEF",
                           XID("myprocedure") XTYPEVOID XSCE)
                       XB("PROCDEF",
                           XID("myprocedure2")
                           XSIMPLESTYPE("private", "int64", "1")
                           XSCE)));

    QTest::newRow("declarationAndprocedure")
        << QString("private int myInt; void myprocedure(){}")
        << QString(XB2("MODULE", "PROGRAM",
                      XDECL(
                           XSIMPLESTYPE("private", "int64", "0"),
                           XID("myInt") NODIMENSIONS)
                      XB("PROCDEF",
                          XID("myprocedure") XTYPEVOID XSCE)));

    QTest::newRow("declarationsAndprocedure")
        << QString("public bool myBool = true; private int64[[2]] myInt (2, 3);"
                   "void myprocedure(){}")
        << QString(XB2("MODULE", "PROGRAM",
                       XDECL(
                           XSIMPLEPTYPE("bool", "0"),
                           XID("myBool") NODIMENSIONS XBOOL(true))
                       XDECL(
                           XSIMPLESTYPE("private", "int64", "2"),
                           XID("myInt") XB("DIMENSIONS", XINT(2) XINT(3)))
                       XB("PROCDEF",
                           XID("myprocedure") XTYPEVOID XSCE)));

    QTest::newRow("procedureWithParam")
        << QString("void myprocedure(private int i){}")
        << QString(XB3("MODULE", "PROGRAM", "PROCDEF",
                       XID("myprocedure") XTYPEVOID XSCE
                       XDECL(XSIMPLESTYPE("private", "int64", "0"),
                             XID("i") NODIMENSIONS)
                  ));

    QTest::newRow("procedureWithParams")
        << QString("void myprocedure(private int i, public bool j){}")
        << QString(XB3("MODULE", "PROGRAM", "PROCDEF",
                       XID("myprocedure") XTYPEVOID XSCE
                       XDECL(XSIMPLESTYPE("private", "int64", "0"), XID("i") NODIMENSIONS)
                       XDECL(XSIMPLEPTYPE("bool", "0"), XID("j") NODIMENSIONS)
                  ));
}

void TestParse::testGlobalDecls_data() {
    ADD_STANDARD_COLUMNS;

    QString iSkeleton("%1 myVar; void myprocedure(){}");
    QString oSkeleton(XB2("MODULE", "PROGRAM",
                         XDECL("%1", XID("myVar") NODIMENSIONS)
                         XB("PROCDEF",
                             XID("myprocedure") XTYPEVOID XSCE)));
    QString ovSkeleton(XB2("MODULE", "PROGRAM",
                          XDECL("%1", XID("myVar") NODIMENSIONS)
                          XB("PROCDEF",
                              XID("myprocedure") XTYPEVOID XSCE)));

    QTest::newRow("privateBool") << iSkeleton.arg("private bool")
                                 << oSkeleton.arg(XSIMPLESTYPE("private", "bool", "0"));
    QTest::newRow("privateInt") << iSkeleton.arg("private int")
                                << oSkeleton.arg(XSIMPLESTYPE("private", "int64", "0"));
    QTest::newRow("publicBool") << iSkeleton.arg("public bool")
                                << oSkeleton.arg(XSIMPLEPTYPE("bool", "0"));
    QTest::newRow("publicInt") << iSkeleton.arg("public int")
                               << oSkeleton.arg(XSIMPLEPTYPE("int64", "0"));
    QTest::newRow("publicString") << iSkeleton.arg("public string")
                                  << oSkeleton.arg(XSIMPLEPTYPE("string", "0"));

    QTest::newRow("privateBoolV") << iSkeleton.arg("private bool[[1]]")
        << ovSkeleton.arg(XSIMPLESTYPE("private", "bool", "1"));
    QTest::newRow("privateIntV") << iSkeleton.arg("private int[[2]]")
        << ovSkeleton.arg(XSIMPLESTYPE("private", "int64", "2"));
    QTest::newRow("publicBoolV") << iSkeleton.arg("public bool[[1]]")
        << ovSkeleton.arg(XSIMPLEPTYPE("bool", "1"));
    QTest::newRow("publicIntV") << iSkeleton.arg("public int[[2]]")
        << ovSkeleton.arg(XSIMPLEPTYPE("int64", "2"));
    QTest::newRow("publicStringV") << iSkeleton.arg("public string [[1]]")
        << ovSkeleton.arg(XSIMPLEPTYPE("string", "1"));
}

void TestParse::testStmtCompound_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("compoundEmpty")
        << QString(SIMPLE(""))
        << QString(SIMPLE_PARSE(XSCE));

    QTest::newRow("compoundOne")
        << QString(SIMPLE("{1;}"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_EXPR", XINT(1))));

    QTest::newRow("compoundTwo")
        << QString(SIMPLE("true;false;"))
        << QString(SIMPLE_PARSE_STMTS(
                                   XB("STMT_EXPR", XBOOL(true))
                                   XB("STMT_EXPR", XBOOL(false))));

    QTest::newRow("compoundThree")
        << QString(SIMPLE("1;2;3;"))
        << QString(SIMPLE_PARSE_STMTS(
                                   XB("STMT_EXPR", XINT(1))
                                   XB("STMT_EXPR", XINT(2))
                                   XB("STMT_EXPR", XINT(3))));

    QTest::newRow("compoundDecl")
        << QString(SIMPLE("1;{public int a;;}3;"))
        << QString(SIMPLE_PARSE_STMTS(
                                   XB("STMT_EXPR", XINT(1))
                                   XB("STMT_COMPOUND",
                                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("a") NODIMENSIONS))
                                   XB("STMT_EXPR", XINT(3))));
}

void TestParse::testStmtIf_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("ifOnly")
        << QString(SIMPLE("if(1);"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_IF", XINT(1) XSCE)));

    QTest::newRow("ifElse")
        << QString(SIMPLE("if(1);else;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_IF", XINT(1) XSCE XSCE)));
}

void TestParse::testStmtFor_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("forFull")
        << QString(SIMPLE("for(1;2;3);"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_FOR", XINT(1) XINT(2) XINT(3) XSCE)));

    QTest::newRow("forEmpty")
        << QString(SIMPLE("for(;;);"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_FOR", "<EXPR_NONE/><EXPR_NONE/>"
                                               "<EXPR_NONE/>" XSCE)));
}

void TestParse::testStmtWhile_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("whileHello")
        << QString(SIMPLE("while(\"hello\");"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_WHILE", XSTR("hello") XSCE)));
}

void TestParse::testStmtDoWhile_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("doWhileHello")
        << QString(SIMPLE("do; while(\"hello\");"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_DOWHILE", XSCE XSTR("hello"))));
}

void TestParse::testStmtOther_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("returnExpression")
        << QString(SIMPLE("return 42;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_RETURN", XINT(42))));

    QTest::newRow("return")
        << QString(SIMPLE("return;"))
        << QString(SIMPLE_PARSE_STMTS("<STMT_RETURN/>"));

    QTest::newRow("continue")
        << QString(SIMPLE("for(;;) continue;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_FOR", "<EXPR_NONE/><EXPR_NONE/>"
                                   "<EXPR_NONE/><STMT_CONTINUE/>")));

    QTest::newRow("break")
        << QString(SIMPLE("for(;;) break;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_FOR", "<EXPR_NONE/><EXPR_NONE/>"
                                   "<EXPR_NONE/><STMT_BREAK/>")));

    QTest::newRow("statementExpression")
        << QString(SIMPLE("42;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_EXPR", XINT(42))));

    QTest::newRow("emptyExpressionEnd")
        << QString(SIMPLE("42;;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_EXPR", XINT(42))));

    QTest::newRow("emptyExpressionMiddle")
        << QString(SIMPLE("42;;42;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_EXPR", XINT(42))
                                       XB("STMT_EXPR", XINT(42))));

    // TODO: fix this test, or improve parser to again account for this case
    QTest::newRow("emptyExpressionBegin")
        << QString(SIMPLE(";42;"))
        << QString(SIMPLE_PARSE_STMTS(XB("STMT_EXPR", XINT(42))));
}

void TestParse::testExprPrimary_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("literalDecimal")
        << QString(SIMPLE_E("42"))
        << QString(SIMPLE_PARSE_E(XINT(42)));

    QTest::newRow("literalString")
        << QString(SIMPLE_E("\"Hello\\n\""))
        << QString(SIMPLE_PARSE_E(XSTR("Hello\\n")));

    QTest::newRow("literalBoolTrue")
        << QString(SIMPLE_E("true"))
        << QString(SIMPLE_PARSE_E(XBOOL(true)));

    QTest::newRow("literalBoolFalse")
        << QString(SIMPLE_E("false"))
        << QString(SIMPLE_PARSE_E(XBOOL(false)));

    QTest::newRow("identifier")
        << QString(SIMPLE_E("identifier"))
        << QString(SIMPLE_PARSE_E(XRVAR("identifier")));

    QTest::newRow("identifierInBrackets")
        << QString(SIMPLE_E("(identifier)"))
        << QString(SIMPLE_PARSE_E(XRVAR("identifier")));
}

void TestParse::testExprPostfix_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("procedureCall")
        << QString(SIMPLE_E("f()"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_PROCCALL", XID("f"))));

    QTest::newRow("procedureCallWithArg")
        << QString(SIMPLE_E("f(42)"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_PROCCALL", XID("f") XINT(42))));

    QTest::newRow("procedureCallWithArgs")
        << QString(SIMPLE_E("f(42,24)"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_PROCCALL", XID("f") XINT(42) XINT(24))));

    QTest::newRow("matrixExpressionWildCard")
        << QString(SIMPLE_E("m[:]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_INDEX",
                XRVAR("m") XB2("SUBSCRIPT", "INDEX_SLICE", XNONE XNONE))));

    QTest::newRow("matrixExpressionWildCards")
        << QString(SIMPLE_E("m[:, :]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_INDEX",
                XRVAR("m") XB("SUBSCRIPT",
                  XB("INDEX_SLICE", XNONE XNONE)
                  XB("INDEX_SLICE", XNONE XNONE)))));

    QTest::newRow("matrixExpression")
        << QString(SIMPLE_E("m[42]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_INDEX",
                XRVAR("m") XB("SUBSCRIPT", XB("INDEX_INT", XINT(42))))));

    QTest::newRow("matrixExpressions")
        << QString(SIMPLE_E("m[42,24]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_INDEX",
                XRVAR("m") XB("SUBSCRIPT",
          XB("INDEX_INT", XINT(42))
          XB("INDEX_INT", XINT(24))))));

    QTest::newRow("postfixExpression")
        << QString(SIMPLE_E("m[:, 42, :]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_INDEX",
                XRVAR("m") XB("SUBSCRIPT",
                  XB("INDEX_SLICE", XNONE XNONE)
          XB("INDEX_INT", XINT(42))
                  XB("INDEX_SLICE", XNONE XNONE)
        ))));
}

void TestParse::testExprUnary_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("unaryMinus")
        << QString(SIMPLE_E("-42"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_UMINUS", XINT(42))));

    QTest::newRow("unaryPrefixDecrement")
        << QString(SIMPLE_E("--x"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_PREFIX_DEC", XB("LVALUE", XID("x")))));

    QTest::newRow("unaryPrefixIncrement")
        << QString(SIMPLE_E("++x"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_PREFIX_INC", XB("LVALUE", XID("x")))));


    QTest::newRow("unaryMinusMinus")
        << QString(SIMPLE_E("-(-42)"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_UMINUS", XINT(42))));

    QTest::newRow("unaryNeg")
        << QString(SIMPLE_E("!42"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_UNEG", XINT(42))));

    QTest::newRow("unaryNegNeg")
        << QString(SIMPLE_E("!!42"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_UNEG", XINT(42))));

    QTest::newRow("unaryMinusNeg")
        << QString(SIMPLE_E("-!42"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_UNEG", XINT(42))));

    QTest::newRow("unaryNegMinus")
        << QString(SIMPLE_E("!-42"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_UMINUS", XINT(42))));
}

void TestParse::testExprCast_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("castIntToBool")
        << QString(SIMPLE_E("(bool) 42"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST", XTYPEDATAF("bool") XINT(42))));
}

void TestParse::testExprMult_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("multiplication", "*", "MUL");
    ADD_ROWS_SINGLE_BINARY_LEFT("division",       "/", "DIV");
    ADD_ROWS_SINGLE_BINARY_LEFT("modulo",         "%", "MOD");
}

void TestParse::testExprAddi_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("addition",    "+", "ADD");
    ADD_ROWS_SINGLE_BINARY_LEFT("subtraction", "-", "SUB");
}

void TestParse::testExprRela_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("lessEqual",    "<=", "LE");
    ADD_ROWS_SINGLE_BINARY_LEFT("greaterEqual", ">=", "GE");
    ADD_ROWS_SINGLE_BINARY_LEFT("less",         "<",  "LT");
    ADD_ROWS_SINGLE_BINARY_LEFT("greater",      ">",  "GT");
}

void TestParse::testExprEqua_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("equal",    "==", "EQ");
    ADD_ROWS_SINGLE_BINARY_LEFT("notEqual", "!=", "NE");
}

void TestParse::testExprLAnd_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("logicalAnd", "&&", "LAND");
}

void TestParse::testExprLOr_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("logicalOr", "||", "LOR");
}

void TestParse::testExprCond_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("ternaryConditional")
        << QString(SIMPLE_E("o1 ? o2 : o3"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF", XRVAR("o1") XRVAR("o2") XRVAR("o3"))));

    QTest::newRow("ternaryConditionalAssoc")
        << QString(SIMPLE_E("o1 ? o2 ? o3 : o4 : o5"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XRVAR("o1")
                                      XB("EXPR_TERNIF",
                                         XRVAR("o2") XRVAR("o3") XRVAR("o4"))
                                      XRVAR("o5"))));

    QTest::newRow("ternaryConditionalAssoc")
        << QString(SIMPLE_E("o1 ? o2 : o3 ? o4 : o5"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XRVAR("o1") XRVAR("o2")
                                      XB("EXPR_TERNIF",
                                          XRVAR("o3") XRVAR("o4") XRVAR("o5")))));
}

void TestParse::testExprAssign_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_ASSIGN("assign",    "=",  "ASSIGN");
    ADD_ROWS_ASSIGN("mulAssign", "*=", "ASSIGN_MUL");
    ADD_ROWS_ASSIGN("divAssign", "/=", "ASSIGN_DIV");
    ADD_ROWS_ASSIGN("modAssign", "%=", "ASSIGN_MOD");
    ADD_ROWS_ASSIGN("addAssign", "+=", "ASSIGN_ADD");
    ADD_ROWS_ASSIGN("subAssign", "-=", "ASSIGN_SUB");
}

void TestParse::testExprPrecedence_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("ternaryToLor1")
        << QString(SIMPLE_E("o1 || o2 ? o3 : o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XB("EXPR_BINARY_LOR", XRVAR("o1") XRVAR("o2"))
                                      XRVAR("o3") XRVAR("o4"))));

    QTest::newRow("ternaryToLor2")
        << QString(SIMPLE_E("o1 ? o2 || o3 : o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XRVAR("o1")
                                      XB("EXPR_BINARY_LOR", XRVAR("o2") XRVAR("o3"))
                                      XRVAR("o4"))));

    QTest::newRow("ternaryToLor3")
        << QString(SIMPLE_E("o1 ? o2 : o3 || o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XRVAR("o1") XRVAR("o2")
                                      XB("EXPR_BINARY_LOR", XRVAR("o3") XRVAR("o4")))));

    ADD_ROWS_BINARY_RIGHT("lorToLand", "||", "LOR",  "&&", "LAND");
    ADD_ROWS_BINARY_RIGHT("landToEq",  "&&", "LAND", "==", "EQ");
    ADD_ROWS_BINARY_RIGHT("landToNe",  "&&", "LAND", "!=", "NE");
    ADD_ROWS_BINARY_RIGHT("eqToLe",    "==", "EQ",   "<=", "LE");
    ADD_ROWS_BINARY_RIGHT("eqToGe",    "==", "EQ",   ">=", "GE");
    ADD_ROWS_BINARY_RIGHT("eqToLt",    "==", "EQ",   "<",  "LT");
    ADD_ROWS_BINARY_RIGHT("eqToGt",    "==", "EQ",   ">",  "GT");
    ADD_ROWS_BINARY_RIGHT("neToLe",    "!=", "NE",   "<=", "LE");
    ADD_ROWS_BINARY_RIGHT("neToGe",    "!=", "NE",   ">=", "GE");
    ADD_ROWS_BINARY_RIGHT("neToLt",    "!=", "NE",   "<",  "LT");
    ADD_ROWS_BINARY_RIGHT("neToGt",    "!=", "NE",   ">",  "GT");
    ADD_ROWS_BINARY_RIGHT("leToAdd",   "<=", "LE",   "+",  "ADD");
    ADD_ROWS_BINARY_RIGHT("leToSub",   "<=", "LE",   "-",  "SUB");
    ADD_ROWS_BINARY_RIGHT("geToAdd",   ">=", "GE",   "+",  "ADD");
    ADD_ROWS_BINARY_RIGHT("geToSub",   ">=", "GE",   "-",  "SUB");
    ADD_ROWS_BINARY_RIGHT("ltToAdd",   "<",  "LT",   "+",  "ADD");
    ADD_ROWS_BINARY_RIGHT("ltToSub",   "<",  "LT",   "-",  "SUB");
    ADD_ROWS_BINARY_RIGHT("gtToAdd",   ">",  "GT",   "+",  "ADD");
    ADD_ROWS_BINARY_RIGHT("gtToSub",   ">",  "GT",   "-",  "SUB");
    ADD_ROWS_BINARY_RIGHT("addToMul",  "+",  "ADD",  "*",  "MUL");
    ADD_ROWS_BINARY_RIGHT("addToDiv",  "+",  "ADD",  "/",  "DIV");
    ADD_ROWS_BINARY_RIGHT("addToMod",  "+",  "ADD",  "%",  "MOD");
    ADD_ROWS_BINARY_RIGHT("subToMul",  "-",  "SUB",  "*",  "MUL");
    ADD_ROWS_BINARY_RIGHT("subToDiv",  "-",  "SUB",  "/",  "DIV");
    ADD_ROWS_BINARY_RIGHT("subToMod",  "-",  "SUB",  "%",  "MOD");

    QTest::newRow("mulToCast1") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("(int) o1 * o2"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_BINARY_MUL",
                                      XB("EXPR_CAST",
                                          XTYPEDATAF("int64") XRVAR("o1"))
                                      XRVAR("o2"))));

    QTest::newRow("mulToCast2") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("o1 * (int) o2"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_BINARY_MUL",
                                      XRVAR("o1")
                                      XB("EXPR_CAST",
                                          XTYPEDATAF("int64") XRVAR("o2")))));

    QTest::newRow("castToUminus") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("(bool) - (int) ! o1"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST",
                                      XTYPEDATAF("bool")
                                      XB2("EXPR_UMINUS", "EXPR_CAST",
                                          XTYPEDATAF("int64")
                                          XB("EXPR_UNEG", XRVAR("o1"))))));

    QTest::newRow("castToUneg") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("(bool) ! (int) - o1"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST",
                                      XTYPEDATAF("bool")
                                      XB2("EXPR_UNEG", "EXPR_CAST",
                                          XTYPEDATAF("int64")
                                          XB("EXPR_UMINUS", XRVAR("o1"))))));

    QTest::newRow("uminusToPROCCALL1")
        << QString(SIMPLE_E("-f()"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_PROCCALL", XID("f"))));

    QTest::newRow("unegToPROCCALL1")
        << QString(SIMPLE_E("!f()"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_PROCCALL", XID("f"))));

    QTest::newRow("uminusToPROCCALL2")
        << QString(SIMPLE_E("-f(o1,o2)"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_PROCCALL",
                                      XID("f") XRVAR("o1") XRVAR("o2"))));

    QTest::newRow("unegToPROCCALL2")
        << QString(SIMPLE_E("!f(o1,o2)"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_PROCCALL",
                                      XID("f") XRVAR("o1") XRVAR("o2"))));

    QTest::newRow("uminusToWildcard") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("-o1[:]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_INDEX",
                                      XRVAR("o1") XB2("SUBSCRIPT", "INDEX_SLICE", XNONE XNONE))));

    QTest::newRow("unegToWildcard") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("!o1[:]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_INDEX",
                                      XRVAR("o1") XB2("SUBSCRIPT", "INDEX_SLICE", XNONE XNONE))));

    QTest::newRow("uminusToSubscript") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("-o1[o2]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_INDEX",
                                      XRVAR("o1") XB2("SUBSCRIPT", "INDEX_INT", XRVAR("o2")))));

    QTest::newRow("unegToSubscript") /// \todo RVAR or LVAR
        << QString(SIMPLE_E("!o1[o2]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_INDEX",
                                      XRVAR("o1") XB2("SUBSCRIPT", "INDEX_INT", XRVAR("o2")))));
}

void TestParse::testInlineDecls_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("simpleDeclBefore")
        << QString(SIMPLE("public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleDeclAfter")
        << QString(SIMPLE("f(); public int i;;"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                  )));

    QTest::newRow("simpleDeclMiddle")
        << QString(SIMPLE("f(); public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleDeclsMiddle")
        << QString(SIMPLE("f(); public int i; f(); public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleInlineDecl")
        << QString(SIMPLE("{ public int i; f(); } f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XSTMTS(
                          XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                          XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f")))
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleInlineDeclLonely")
        << QString(SIMPLE("{ public int i;; } f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XSTMTS(
                          XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS))
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleInlineDeclAfter")
        << QString(SIMPLE("f(); { public int i; f(); }"))
        << QString(SIMPLE_PARSE_STMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XSTMTS(XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f")))
                  ));

    QTest::newRow("simpleInlineDeclLonelyAfter")
        << QString(SIMPLE("f(); { public int i;; }"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XSTMTS(XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS))
                  )));

    QTest::newRow("simpleInlineDeclMiddle")
        << QString(SIMPLE("f(); { public int i; f(); } f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XSTMTS(
                          XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS)
                          XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f")))
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));

    QTest::newRow("simpleInlineDeclLonelyMiddle")
        << QString(SIMPLE("f(); { public int i;; } f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                      XSTMTS(
                          XDECL(XSIMPLEPTYPE("int64", "0"), XID("i") NODIMENSIONS))
                      XB2("STMT_EXPR", "EXPR_PROCCALL", XID("f"))
                  )));
}

QTEST_APPLESS_MAIN(TestParse)
