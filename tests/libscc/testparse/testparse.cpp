#include <libscc/parser.h>
#include <libscc/treenode.h>
#include <QtTest>
#include <string>


#define ADD_STANDARD_COLUMNS \
    QTest::addColumn<QString>("sccInput");\
    QTest::addColumn<QString>("xmlOutput")

// XML elements with contents:
#define XB(a,b) "<" a ">" b "</" a ">"
#define XB2(a,b,c) "<" a "><" b ">" c "</" b "></" a ">"
#define XB3(a,b,c,d) "<" a "><" b "><" c ">" d "</" c "></" b "></" a ">"

// Parser-specific XML elements:
#define XID(a) "<IDENTIFIER value=\"string:" a "\"/>"
#define XTYPE(a) "<TYPE value=\"type:" a "\"/>"
#define XVTYPE(a) "<VTYPE value=\"vtype:" a "\"/>"
#define XBOOL(a) "<BOOL value=\"bool:" #a "\"/>"
#define XINT(a) "<INT value=\"int:" #a "\"/>"
#define XSTR(a) "<STRING value=\"string:&quot;" a "&quot;\"/>"
#define XSCE "<STMT_COMPOUND/>"
#define XSTMTS(a) "<STMT_COMPOUND>" a "</STMT_COMPOUND>"

// Macros for testing simple code in blocks:
#define SIMPLE(x) "public void myfunction() {" x "}"
#define SIMPLE_PARSE(x) XB3("PROGRAM", "FUNDEFS", "FUNDEF",\
        XID("myfunction") XTYPE("public void") x)

// Macros for testing expressions as a statement:
#define SIMPLE_E(x) SIMPLE(x ";")
#define SIMPLE_PARSE_E(x) SIMPLE_PARSE(XB("STMT_EXPR",x))

// Macros to test binary operators:
#define SIMPLE_BINARY_DETAIL(a,b,c) SIMPLE_E(a b c)
#define SIMPLE_BINARY(a) SIMPLE_BINARY_DETAIL("o1",a,"o2")
#define SIMPLE_PARSE_BINARY_DETAIL(a,b,c) SIMPLE_PARSE_E(XB("EXPR_" b, XID(a) XID(c)))
#define SIMPLE_PARSE_BINARY(a) SIMPLE_PARSE_BINARY_DETAIL("o1",a,"o2")

// Macros to test the associativity and precedence of binary operators:
#define SIMPLE_ASSOC_DETAIL(a,b,c,d,e) SIMPLE_E(a b c d e)
#define SIMPLE_ASSOC(a,b) SIMPLE_ASSOC_DETAIL("o1",a,"o2",b,"o3")
#define SIMPLE_PARSE_ASSOC_LEFT_DETAIL(a,b,c,d,e) \
    SIMPLE_PARSE_E(XB("EXPR_" d, XB("EXPR_" b, XID(a) XID(c)) XID(e)))
#define SIMPLE_PARSE_ASSOC_LEFT(a,b) SIMPLE_PARSE_ASSOC_LEFT_DETAIL("o1",a,"o2",b,"o3")
#define SIMPLE_PARSE_ASSOC_RIGHT_DETAIL(a,b,c,d,e) \
    SIMPLE_PARSE_E(XB("EXPR_" b, XID(a) XB("EXPR_" d, XID(c) XID(e))))
#define SIMPLE_PARSE_ASSOC_RIGHT(a,b) SIMPLE_PARSE_ASSOC_RIGHT_DETAIL("o1",a,"o2",b,"o3")
#define ADD_ROWS_SINGLE_BINARY_LEFT(a,b,c) \
    QTest::newRow(a) << QString(SIMPLE_BINARY(b))\
                     << QString(SIMPLE_PARSE_BINARY(c));\
    QTest::newRow(a "Assoc") << QString(SIMPLE_ASSOC(b, b))\
                             << QString(SIMPLE_PARSE_ASSOC_LEFT(c, c));
#define ADD_ROWS_SINGLE_BINARY_RIGHT(a,b,c) \
    QTest::newRow(a) << QString(SIMPLE_BINARY(b))\
                     << QString(SIMPLE_PARSE_BINARY(c));\
    QTest::newRow(a "Assoc") << QString(SIMPLE_ASSOC(b, b))\
                             << QString(SIMPLE_PARSE_ASSOC_RIGHT(c, c));
#define ADD_ROWS_BINARY_RIGHT(a,b,c,d,e) \
    QTest::newRow(a) << QString(SIMPLE_ASSOC(b, d))\
                     << QString(SIMPLE_PARSE_ASSOC_RIGHT(c, e));\
    QTest::newRow(a "Inverse") << QString(SIMPLE_ASSOC(d, b))\
                               << QString(SIMPLE_PARSE_ASSOC_LEFT(e, c));


class TestParse: public QObject {
    Q_OBJECT

    private:
        void simpleParseTest();

    private slots:
        inline void testParseProgram() { simpleParseTest(); }
        void testParseProgram_data();
        inline void testGlobalDecls() { simpleParseTest(); }
        void testGlobalDecls_data();
        inline void testStmtCompound() { simpleParseTest(); }
        void testStmtCompound_data();
        inline void testStmtIf() { simpleParseTest(); }
        void testStmtIf_data();
        inline void testStmtFor() { simpleParseTest(); }
        void testStmtFor_data();
        inline void testStmtWhile() { simpleParseTest(); }
        void testStmtWhile_data();
        inline void testStmtDoWhile() { simpleParseTest(); }
        void testStmtDoWhile_data();
        inline void testStmtOther() { simpleParseTest(); }
        void testStmtOther_data();
        inline void testExprPrimary() { simpleParseTest(); }
        void testExprPrimary_data();
        inline void testExprPostfix() { simpleParseTest(); }
        void testExprPostfix_data();
        inline void testExprUnary() { simpleParseTest(); }
        void testExprUnary_data();
        inline void testExprCast() { simpleParseTest(); }
        void testExprCast_data();
        inline void testExprMatrix() { simpleParseTest(); }
        void testExprMatrix_data();
        inline void testExprMult() { simpleParseTest(); }
        void testExprMult_data();
        inline void testExprAddi() { simpleParseTest(); }
        void testExprAddi_data();
        inline void testExprRela() { simpleParseTest(); }
        void testExprRela_data();
        inline void testExprEqua() { simpleParseTest(); }
        void testExprEqua_data();
        inline void testExprLAnd() { simpleParseTest(); }
        void testExprLAnd_data();
        inline void testExprLOr() { simpleParseTest(); }
        void testExprLOr_data();
        inline void testExprCond() { simpleParseTest(); }
        void testExprCond_data();
        inline void testExprAssign() { simpleParseTest(); }
        void testExprAssign_data();
        inline void testExprPrecedence() { simpleParseTest(); }
        void testExprPrecedence_data();
        inline void testInlineDecls() { simpleParseTest(); }
        void testInlineDecls_data();
};

void TestParse::simpleParseTest() {
    QFETCH(QString, sccInput);
    QFETCH(QString, xmlOutput);

    TreeNode *n;
    std::string input(sccInput.toStdString());
    QCOMPARE(sccparse_mem(input.c_str(), input.size(), &n), 0);
    QCOMPARE(QString(n->toXml(false).c_str()), xmlOutput);
    delete n;
}

void TestParse::testParseProgram_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("oneFunction")
        << QString("public void myfunction(){}")
        << QString(XB3("PROGRAM", "FUNDEFS", "FUNDEF",
                       XID("myfunction") XTYPE("public void") XSCE));

    QTest::newRow("twoFunctions")
        << QString("public void myfunction(){}private int[][] myfunction2(){}")
        << QString(XB2("PROGRAM", "FUNDEFS",
                       XB("FUNDEF",
                           XID("myfunction") XTYPE("public void") XSCE)
                       XB("FUNDEF",
                           XID("myfunction2") XVTYPE("private int 2") XSCE)));

    QTest::newRow("declarationAndFunction")
        << QString("private int myInt; public void myfunction(){}")
        << QString(XB("PROGRAM",
                      XB2("DECL_GLOBALS", "DECL",
                           XID("myInt") XTYPE("private int"))
                      XB2("FUNDEFS", "FUNDEF",
                          XID("myfunction") XTYPE("public void") XSCE)));

    QTest::newRow("declarationsAndFunction")
        << QString("private int[][] myInt[2][3]; public bool myBool = true;"
                   "public void myfunction(){}")
        << QString(XB("PROGRAM",
                       XB("DECL_GLOBALS",
                           XB("DECL",
                               XID("myInt") XVTYPE("private int 2")
                               XB("DECL_VSUFFIX", XINT(2) XINT(3)))
                           XB("DECL",
                               XID("myBool") XTYPE("public bool") XBOOL(true)))
                       XB2("FUNDEFS", "FUNDEF",
                           XID("myfunction") XTYPE("public void") XSCE)));

    QTest::newRow("functionWithParam")
        << QString("public void myfunction(private int i){}")
        << QString(XB3("PROGRAM", "FUNDEFS", "FUNDEF",
                       XID("myfunction") XTYPE("public void") XSCE
                       XB("FUNDEF_PARAM", XID("i") XTYPE("private int"))
                  ));

    QTest::newRow("functionWithParams")
        << QString("public void myfunction(private int i, public bool j){}")
        << QString(XB3("PROGRAM", "FUNDEFS", "FUNDEF",
                       XID("myfunction") XTYPE("public void") XSCE
                       XB("FUNDEF_PARAM", XID("i") XTYPE("private int"))
                       XB("FUNDEF_PARAM", XID("j") XTYPE("public bool"))
                  ));
}

void TestParse::testGlobalDecls_data() {
    ADD_STANDARD_COLUMNS;

    QString iSkeleton("%1 myVar; public void myfunction(){}");
    QString oSkeleton(XB("PROGRAM",
                         XB2("DECL_GLOBALS", "DECL",
                             XID("myVar") XTYPE("%1"))
                         XB2("FUNDEFS", "FUNDEF",
                             XID("myfunction") XTYPE("public void") XSCE)));
    QString ovSkeleton(XB("PROGRAM",
                          XB2("DECL_GLOBALS", "DECL",
                              XID("myVar") XVTYPE("%1"))
                          XB2("FUNDEFS", "FUNDEF",
                              XID("myfunction") XTYPE("public void") XSCE)));

    QTest::newRow("privateBool") << iSkeleton.arg("private bool")
                                 << oSkeleton.arg("private bool");
    QTest::newRow("privateInt") << iSkeleton.arg("private int")
                                << oSkeleton.arg("private int");
    QTest::newRow("publicBool") << iSkeleton.arg("public bool")
                                << oSkeleton.arg("public bool");
    QTest::newRow("publicInt") << iSkeleton.arg("public int")
                               << oSkeleton.arg("public int");
    QTest::newRow("publicSInt") << iSkeleton.arg("public signed int")
                                << oSkeleton.arg("public int");
    QTest::newRow("publicUInt") << iSkeleton.arg("public unsigned int")
                                << oSkeleton.arg("public unsigned int");
    QTest::newRow("publicString") << iSkeleton.arg("public string")
                                  << oSkeleton.arg("public string");
    QTest::newRow("publicBool_") << iSkeleton.arg("bool")
                                 << oSkeleton.arg("public bool");
    QTest::newRow("publicInt_") << iSkeleton.arg("int")
                                << oSkeleton.arg("public int");
    QTest::newRow("publicSInt_") << iSkeleton.arg("signed int")
                                 << oSkeleton.arg("public int");
    QTest::newRow("publicUInt_") << iSkeleton.arg("unsigned int")
                                 << oSkeleton.arg("public unsigned int");
    QTest::newRow("publicString_") << iSkeleton.arg("string")
                                   << oSkeleton.arg("public string");

    QTest::newRow("privateBoolV") << iSkeleton.arg("private bool[][][]")
                                  << ovSkeleton.arg("private bool 3");
    QTest::newRow("privateIntV") << iSkeleton.arg("private int[]")
                                 << ovSkeleton.arg("private int 1");
    QTest::newRow("publicBoolV") << iSkeleton.arg("public bool[][]")
                                 << ovSkeleton.arg("public bool 2");
    QTest::newRow("publicIntV") << iSkeleton.arg("public int[][][][][]")
                                << ovSkeleton.arg("public int 5");
    QTest::newRow("publicSIntV") << iSkeleton.arg("public signed int[][][][][][]")
                                 << ovSkeleton.arg("public int 6");
    QTest::newRow("publicUIntV") << iSkeleton.arg("public unsigned int[][][][][][][]")
                                 << ovSkeleton.arg("public unsigned int 7");
    QTest::newRow("publicStringV") << iSkeleton.arg("public string []")
                                   << ovSkeleton.arg("public string 1");
    QTest::newRow("publicBool_V") << iSkeleton.arg("bool  [] []")
                                  << ovSkeleton.arg("public bool 2");
    QTest::newRow("publicInt_V") << iSkeleton.arg("int[]")
                                 << ovSkeleton.arg("public int 1");
    QTest::newRow("publicSInt_V") << iSkeleton.arg("signed int[][]")
                                  << ovSkeleton.arg("public int 2");
    QTest::newRow("publicUInt_V") << iSkeleton.arg("unsigned int[]")
                                  << ovSkeleton.arg("public unsigned int 1");
    QTest::newRow("publicString_V") << iSkeleton.arg("string[]")
                                    << ovSkeleton.arg("public string 1");
}

void TestParse::testStmtCompound_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("compoundEmpty")
        << QString(SIMPLE(""))
        << QString(SIMPLE_PARSE(XSCE));

    QTest::newRow("compoundOne")
        << QString(SIMPLE("{1;}"))
        << QString(SIMPLE_PARSE(XB("STMT_EXPR", XINT(1))));

    QTest::newRow("compoundTwo")
        << QString(SIMPLE("true;false;"))
        << QString(SIMPLE_PARSE(XSTMTS(
                                   XB("STMT_EXPR", XBOOL(true))
                                   XB("STMT_EXPR", XBOOL(false)))));

    QTest::newRow("compoundThree")
        << QString(SIMPLE("1;2;3;"))
        << QString(SIMPLE_PARSE(XSTMTS(
                                   XB("STMT_EXPR", XINT(1))
                                   XB("STMT_EXPR", XINT(2))
                                   XB("STMT_EXPR", XINT(3)))));

    QTest::newRow("compoundDecl")
        << QString(SIMPLE("1;{public int a;}3;"))
        << QString(SIMPLE_PARSE(XSTMTS(
                                   XB("STMT_EXPR", XINT(1))
                                   XB("STMT_COMPOUND",
                                      XB("DECL", XID("a") XTYPE("public int")))
                                   XB("STMT_EXPR", XINT(3)))));
}

void TestParse::testStmtIf_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("ifOnly")
        << QString(SIMPLE("if(1);"))
        << QString(SIMPLE_PARSE(XB("STMT_IF", XINT(1) XSCE)));

    QTest::newRow("ifElse")
        << QString(SIMPLE("if(1);else;"))
        << QString(SIMPLE_PARSE(XB("STMT_IF", XINT(1) XSCE XSCE)));
}

void TestParse::testStmtFor_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("forFull")
        << QString(SIMPLE("for(1;2;3);"))
        << QString(SIMPLE_PARSE(XB("STMT_FOR", XINT(1) XINT(2) XINT(3) XSCE)));

    QTest::newRow("forEmpty")
        << QString(SIMPLE("for(;;);"))
        << QString(SIMPLE_PARSE(XB("STMT_FOR", "<EXPR_NONE/><EXPR_NONE/>"
                                               "<EXPR_NONE/>" XSCE)));
}

void TestParse::testStmtWhile_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("whileHello")
        << QString(SIMPLE("while(\"hello\");"))
        << QString(SIMPLE_PARSE(XB("STMT_WHILE", XSTR("hello") XSCE)));
}

void TestParse::testStmtDoWhile_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("doWhileHello")
        << QString(SIMPLE("do; while(\"hello\")"))
        << QString(SIMPLE_PARSE(XB("STMT_DOWHILE", XSCE XSTR("hello"))));
}

void TestParse::testStmtOther_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("returnExpression")
        << QString(SIMPLE("return 42;"))
        << QString(SIMPLE_PARSE(XB("STMT_RETURN", XINT(42))));

    QTest::newRow("return")
        << QString(SIMPLE("return;"))
        << QString(SIMPLE_PARSE("<STMT_RETURN/>"));

    QTest::newRow("continue")
        << QString(SIMPLE("continue;"))
        << QString(SIMPLE_PARSE("<STMT_CONTINUE/>"));

    QTest::newRow("break")
        << QString(SIMPLE("break;"))
        << QString(SIMPLE_PARSE("<STMT_BREAK/>"));

    QTest::newRow("statementExpression")
        << QString(SIMPLE("42;"))
        << QString(SIMPLE_PARSE(XB("STMT_EXPR", XINT(42))));

    QTest::newRow("emptyExpression")
        << QString(SIMPLE("42;;"))
        << QString(SIMPLE_PARSE(XSTMTS(XB("STMT_EXPR", XINT(42)) XSCE)));

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
        << QString(SIMPLE_PARSE_E(XID("identifier")));

    QTest::newRow("identifierInBrackets")
        << QString(SIMPLE_E("(identifier)"))
        << QString(SIMPLE_PARSE_E(XID("identifier")));
}

void TestParse::testExprPostfix_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("functionCall")
        << QString(SIMPLE_E("f()"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_FUNCALL", XID("f"))));

    QTest::newRow("functionCallWithArg")
        << QString(SIMPLE_E("f(42)"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_FUNCALL", XID("f") XINT(42))));

    QTest::newRow("functionCallWithArgs")
        << QString(SIMPLE_E("f(42,24)"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_FUNCALL", XID("f") XINT(42) XINT(24))));

    QTest::newRow("matrixExpressionWildCard")
        << QString(SIMPLE_E("m[*]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_WILDCARD", XID("m"))));

    QTest::newRow("matrixExpressionWildCards")
        << QString(SIMPLE_E("m[*][*]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_WILDCARD", "EXPR_WILDCARD", XID("m"))));

    QTest::newRow("matrixExpression")
        << QString(SIMPLE_E("m[42]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_SUBSCRIPT", XID("m") XINT(42))));

    QTest::newRow("matrixExpressions")
        << QString(SIMPLE_E("m[42][24]"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_SUBSCRIPT",
                                      XB("EXPR_SUBSCRIPT", XID("m") XINT(42))
                                      XINT(24))));

    QTest::newRow("postfixExpression")
        << QString(SIMPLE_E("m[*](42)[42]()"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_FUNCALL", "EXPR_SUBSCRIPT",
                                      XB("EXPR_FUNCALL",
                                          XB("EXPR_WILDCARD", XID("m"))
                                          XINT(42))
                                      XINT(42))));
}

void TestParse::testExprUnary_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("unaryMinus")
        << QString(SIMPLE_E("-42"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_UMINUS", XINT(42))));

    QTest::newRow("unaryMinusMinus")
        << QString(SIMPLE_E("--42"))
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

    QTest::newRow("castToPrivateString")
        << QString(SIMPLE_E("(private bool) 42"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST", XTYPE("private bool") XINT(42))));
}

void TestParse::testExprMatrix_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_LEFT("matrixMultiplication", "#", "MATRIXMUL");
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
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF", XID("o1") XID("o2") XID("o3"))));

    QTest::newRow("ternaryConditionalAssoc")
        << QString(SIMPLE_E("o1 ? o2 ? o3 : o4 : o5"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XID("o1")
                                      XB("EXPR_TERNIF",
                                         XID("o2") XID("o3") XID("o4"))
                                      XID("o5"))));

    QTest::newRow("ternaryConditionalAssoc")
        << QString(SIMPLE_E("o1 ? o2 : o3 ? o4 : o5"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XID("o1") XID("o2")
                                      XB("EXPR_TERNIF",
                                          XID("o3") XID("o4") XID("o5")))));
}

void TestParse::testExprAssign_data() {
    ADD_STANDARD_COLUMNS;

    ADD_ROWS_SINGLE_BINARY_RIGHT("assign",    "=",  "ASSIGN");
    ADD_ROWS_SINGLE_BINARY_RIGHT("mulAssign", "*=", "ASSIGN_MUL");
    ADD_ROWS_SINGLE_BINARY_RIGHT("divAssign", "/=", "ASSIGN_DIV");
    ADD_ROWS_SINGLE_BINARY_RIGHT("modAssign", "%=", "ASSIGN_MOD");
    ADD_ROWS_SINGLE_BINARY_RIGHT("addAssign", "+=", "ASSIGN_ADD");
    ADD_ROWS_SINGLE_BINARY_RIGHT("subAssign", "-=", "ASSIGN_SUB");
}

void TestParse::testExprPrecedence_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("ternaryToLor1")
        << QString(SIMPLE_E("o1 || o2 ? o3 : o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XB("EXPR_LOR", XID("o1") XID("o2"))
                                      XID("o3") XID("o4"))));

    QTest::newRow("ternaryToLor2")
        << QString(SIMPLE_E("o1 ? o2 || o3 : o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XID("o1")
                                      XB("EXPR_LOR", XID("o2") XID("o3"))
                                      XID("o4"))));

    QTest::newRow("ternaryToLor3")
        << QString(SIMPLE_E("o1 ? o2 : o3 || o4"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_TERNIF",
                                      XID("o1") XID("o2")
                                      XB("EXPR_LOR", XID("o3") XID("o4")))));

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
    ADD_ROWS_BINARY_RIGHT("mulToMat",  "*",  "MUL",  "#",  "MATRIXMUL");
    ADD_ROWS_BINARY_RIGHT("divToMat",  "/",  "DIV",  "#",  "MATRIXMUL");
    ADD_ROWS_BINARY_RIGHT("modToMat",  "%",  "MOD",  "#",  "MATRIXMUL");

    QTest::newRow("matToCast1")
        << QString(SIMPLE_E("(public void) o1 # o2"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_MATRIXMUL",
                                      XB("EXPR_CAST",
                                          XTYPE("public void") XID("o1"))
                                      XID("o2"))));

    QTest::newRow("matToCast2")
        << QString(SIMPLE_E("o1 # (public void) o2"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_MATRIXMUL",
                                      XID("o1")
                                      XB("EXPR_CAST",
                                          XTYPE("public void") XID("o2")))));

    QTest::newRow("castToUminus")
        << QString(SIMPLE_E("(public void) - (private int) ! o1"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST",
                                      XTYPE("public void")
                                      XB2("EXPR_UMINUS", "EXPR_CAST",
                                          XTYPE("private int")
                                          XB("EXPR_UNEG", XID("o1"))))));

    QTest::newRow("castToUneg")
        << QString(SIMPLE_E("(public void) ! (private int) - o1"))
        << QString(SIMPLE_PARSE_E(XB("EXPR_CAST",
                                      XTYPE("public void")
                                      XB2("EXPR_UNEG", "EXPR_CAST",
                                          XTYPE("private int")
                                          XB("EXPR_UMINUS", XID("o1"))))));

    QTest::newRow("uminusToFuncall1")
        << QString(SIMPLE_E("-f()"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_FUNCALL", XID("f"))));

    QTest::newRow("unegToFuncall1")
        << QString(SIMPLE_E("!f()"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_FUNCALL", XID("f"))));

    QTest::newRow("uminusToFuncall2")
        << QString(SIMPLE_E("-f(o1,o2)"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_FUNCALL",
                                      XID("f") XID("o1") XID("o2"))));

    QTest::newRow("unegToFuncall2")
        << QString(SIMPLE_E("!f(o1,o2)"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_FUNCALL",
                                      XID("f") XID("o1") XID("o2"))));

    QTest::newRow("uminusToWildcard")
        << QString(SIMPLE_E("-o1[*]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_WILDCARD", XID("o1"))));

    QTest::newRow("unegToWildcard")
        << QString(SIMPLE_E("!o1[*]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_WILDCARD", XID("o1"))));

    QTest::newRow("uminusToSubscript")
        << QString(SIMPLE_E("-o1[o2]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UMINUS", "EXPR_SUBSCRIPT",
                                      XID("o1") XID("o2"))));

    QTest::newRow("unegToSubscript")
        << QString(SIMPLE_E("!o1[o2]"))
        << QString(SIMPLE_PARSE_E(XB2("EXPR_UNEG", "EXPR_SUBSCRIPT",
                                      XID("o1") XID("o2"))));
}

void TestParse::testInlineDecls_data() {
    ADD_STANDARD_COLUMNS;

    QTest::newRow("simpleDeclBefore")
        << QString(SIMPLE("public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB("DECL",
                         XID("i") XTYPE("public int"))
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                  )));

    QTest::newRow("simpleDeclAfter")
        << QString(SIMPLE("f(); public int i;"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                      XB("DECL",
                         XID("i") XTYPE("public int"))
                  )));

    QTest::newRow("simpleDeclInline")
        << QString(SIMPLE("f(); public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                      XB("DECL",
                         XID("i") XTYPE("public int"))
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                  )));

    QTest::newRow("simpleDeclsInline")
        << QString(SIMPLE("f(); public int i; f(); public int i; f();"))
        << QString(SIMPLE_PARSE(XSTMTS(
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                      XB("DECL",
                         XID("i") XTYPE("public int"))
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                      XB("DECL",
                         XID("i") XTYPE("public int"))
                      XB2("STMT_EXPR", "EXPR_FUNCALL", XID("f"))
                  )));
}

QTEST_APPLESS_MAIN(TestParse)
#include "testparse.moc"
