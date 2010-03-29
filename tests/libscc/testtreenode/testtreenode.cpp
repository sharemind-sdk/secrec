#include <QtTest>
#include <libscc/secrec/parser.h>
#include <libscc/secrec/treenode.h>


using namespace SecreC;

bool operator==(const YYLTYPE &a, const YYLTYPE &b) {
    if (a.first_line   != b.first_line)   return false;
    if (a.first_column != b.first_column) return false;
    if (a.last_line    != b.last_line)    return false;
    if (a.last_column  != b.last_column)  return false;
    return true;
}

Q_DECLARE_METATYPE(SecrecVarType)
Q_DECLARE_METATYPE(SecrecSecType)

class TestTreeNode: public QObject {
    Q_OBJECT

    private slots:
        void testInitTreeNode();
        void testInitBool();
        void testInitBool_data();
        void testInitInt();
        void testInitInt_data();
        void testInitUInt();
        void testInitUInt_data();
        void testInitString();
        void testInitString_data();
        void testInitType();
        void testInitType_data();
        void testChildren();
};

#define DEFLOC(n,fl,fc,ll,lc) YYLTYPE n;\
    (n).first_line=(fl);\
    (n).first_column=(fc);\
    (n).last_line=(ll);\
    (n).last_column=(lc);

#define DEFLOCCOPY(n,o) YYLTYPE n = o;

#define NR2(a,b) QTest::newRow(#a","#b)<<(a)<<(b)
#define NR3(a,b,c) QTest::newRow(#a","#b","#c)<<(a)<<(b)<<(c)
#define NR4(a,b,c,d) QTest::newRow(#a","#b","#c","#d)<<(a)<<(b)<<(c)<<(d)

void TestTreeNode::testInitTreeNode() {
    DEFLOC(loc,1,2,3,4);
    DEFLOCCOPY(loc2,loc);

    DEFLOC(nloc,4,3,2,1);
    DEFLOCCOPY(nloc2,nloc);

    TreeNode *n = new TreeNode(NODE_INTERNAL_USE, loc);
    QCOMPARE(n->type(), NODE_INTERNAL_USE);
    QCOMPARE(n->location(), loc);
    QCOMPARE(n->children().size(), (unsigned long) 0);

    n->setLocation(nloc);
    QCOMPARE(n->location(), nloc);

    QCOMPARE(loc, loc2);
    QCOMPARE(nloc, nloc2);
    delete n;
    QCOMPARE(loc, loc2);
    QCOMPARE(nloc, nloc2);
}

void TestTreeNode::testInitBool() {
    DEFLOC(loc,1,2,3,4);

    QFETCH(bool, value);

    TreeNodeBool n(value, loc);
    QCOMPARE(n.type(), NODE_LITE_BOOL);
    QCOMPARE(n.value(), value);
    n.setValue(!value);
    QCOMPARE(n.value(), !value);
}

void TestTreeNode::testInitBool_data() {
    QTest::addColumn<bool>("value");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
}

void TestTreeNode::testInitInt() {
    DEFLOC(loc,1,2,3,4);

    QFETCH(int, value1);
    QFETCH(int, value2);

    TreeNodeInt n(value1, loc);
    QCOMPARE(n.type(), NODE_LITE_INT);
    QCOMPARE(n.value(), value1);
    n.setValue(value2);
    QCOMPARE(n.value(), value2);
}

void TestTreeNode::testInitInt_data() {
    QTest::addColumn<int>("value1");
    QTest::addColumn<int>("value2");

    NR2(-42,-42); NR2(-42,-1); NR2(-42,0); NR2(-42,1); NR2(-42,42);
    NR2(-1,-42);  NR2(-1,-1);  NR2(-1,0);  NR2(-1,1);  NR2(-1,42);
    NR2(0,-42);   NR2(0,-1);   NR2(0,0);   NR2(0,1);   NR2(0,42);
    NR2(1,-42);   NR2(1,-1);   NR2(1,0);   NR2(1,1);   NR2(1,42);
    NR2(42,-42);  NR2(42,-1);  NR2(42,0);  NR2(42,1);  NR2(42,42);
}

void TestTreeNode::testInitUInt() {
    DEFLOC(loc,1,2,3,4);

    QFETCH(unsigned, value1);
    QFETCH(unsigned, value2);

    TreeNodeUInt n(value1, loc);
    QCOMPARE(n.type(), NODE_LITE_UINT);
    QCOMPARE(n.value(), value1);
    n.setValue(value2);
    QCOMPARE(n.value(), value2);
}

void TestTreeNode::testInitUInt_data() {
    QTest::addColumn<unsigned>("value1");
    QTest::addColumn<unsigned>("value2");

    NR2(0u,0u); NR2(0u,1u); NR2(0u,42u);
    NR2(1u,0u); NR2(1u,1u); NR2(1u,42u);
    NR2(42u,0u); NR2(42u,1u); NR2(42u,42u);
}

void TestTreeNode::testInitString() {
    DEFLOC(loc,1,2,3,4);

    QFETCH(QString, value1);
    QFETCH(QString, value2);

    TreeNodeString n(value1.toStdString(), loc);
    QCOMPARE(n.type(), NODE_LITE_STRING);
    QCOMPARE(n.value(), value1.toStdString());
    n.setValue(value2.toStdString());
    QCOMPARE(n.value(), value2.toStdString());
}

void TestTreeNode::testInitString_data() {
    QTest::addColumn<QString>("value1");
    QTest::addColumn<QString>("value2");

    NR2("", "");
    NR2("", "NonEmptyString");
    NR2("NonEmptyString", "");
    NR2("NonEmptyString", "NonEmptyString");
}

void TestTreeNode::testInitType() {
    /// \todo
}

void TestTreeNode::testInitType_data() {
    /// \todo
}

void TestTreeNode::testChildren() {
    typedef std::deque<TreeNode*>::size_type ST;
    DEFLOC(loc,1,2,3,4);

    TreeNodeString *n  = new TreeNodeString("n", loc);
    TreeNodeString *c1 = new TreeNodeString("c1", loc);
    TreeNodeString *c2 = new TreeNodeString("c2", loc);
    TreeNodeString *c3 = new TreeNodeString("c3", loc);
    TreeNodeString *c4 = new TreeNodeString("c4", loc);
    QCOMPARE(n->children().size(), (ST) 0);
    n->appendChild(c1);
    QCOMPARE(n->children().size(), (ST) 1);
    QCOMPARE(n->children().at(0).data(), c1);
    n->appendChild(c2);
    QCOMPARE(n->children().size(), (ST) 2);
    QCOMPARE(n->children().at(0).data(), c1);
    QCOMPARE(n->children().at(1).data(), c2);
    n->appendChild(c3);
    QCOMPARE(n->children().size(), (ST) 3);
    QCOMPARE(n->children().at(0).data(), c1);
    QCOMPARE(n->children().at(1).data(), c2);
    QCOMPARE(n->children().at(2).data(), c3);
    n->appendChild(c4);
    QCOMPARE(n->children().size(), (ST) 4);
    QCOMPARE(n->children().at(0).data(), c1);
    QCOMPARE(n->children().at(1).data(), c2);
    QCOMPARE(n->children().at(2).data(), c3);
    QCOMPARE(n->children().at(3).data(), c4);
    delete c2;
    QCOMPARE(n->children().size(), (ST) 4);
    QCOMPARE(n->children().at(0).data(), c1);
    QVERIFY(n->children().at(1).data() == 0);
    QCOMPARE(n->children().at(2).data(), c3);
    QCOMPARE(n->children().at(3).data(), c4);
    delete c4;
    QCOMPARE(n->children().size(), (ST) 4);
    QCOMPARE(n->children().at(0).data(), c1);
    QVERIFY(n->children().at(1).data() == 0);
    QCOMPARE(n->children().at(2).data(), c3);
    QVERIFY(n->children().at(3).data() == 0);
    delete c1;
    QCOMPARE(n->children().size(), (ST) 4);
    QVERIFY(n->children().at(0).data() == 0);
    QVERIFY(n->children().at(1).data() == 0);
    QCOMPARE(n->children().at(2).data(), c3);
    QVERIFY(n->children().at(3).data() == 0);
    delete c3;
    QCOMPARE(n->children().size(), (ST) 4);
    QVERIFY(n->children().at(0).data() == 0);
    QVERIFY(n->children().at(1).data() == 0);
    QVERIFY(n->children().at(2).data() == 0);
    QVERIFY(n->children().at(3).data() == 0);
    delete n;
}

QTEST_APPLESS_MAIN(TestTreeNode)
#include "testtreenode.moc"
