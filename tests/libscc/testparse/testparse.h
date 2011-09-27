#ifndef TEST_PARSE_H
#define TEST_PARSE_H

#include <QtTest>

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

#endif
