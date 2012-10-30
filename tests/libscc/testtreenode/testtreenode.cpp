#include "testtreenode.h"

#include <libscc/parser.h>
#include <libscc/treenode.h>


using namespace SecreC;

#define NR2(a,b) QTest::newRow(#a","#b)<<(a)<<(b)
#define NR3(a,b,c) QTest::newRow(#a","#b","#c)<<(a)<<(b)<<(c)
#define NR4(a,b,c,d) QTest::newRow(#a","#b","#c","#d)<<(a)<<(b)<<(c)<<(d)

void TestTreeNode::testInitTreeNode() {
    SecreC::TreeNode::Location loc(1, 2, 3, 4, "testInitTreeNode");
    SecreC::TreeNode::Location loc2(loc);

    SecreC::TreeNode::Location nloc(4, 3, 2, 1, "testInitTreeNode");
    SecreC::TreeNode::Location nloc2(nloc);

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
    SecreC::TreeNode::Location loc(1, 2, 3, 4, "testInitBool");

    QFETCH(bool, value);

    TreeNodeExprBool n(value, loc);
    QCOMPARE(n.type(), NODE_LITE_BOOL);
    QCOMPARE(n.value(), value);
}

void TestTreeNode::testInitBool_data() {
    QTest::addColumn<bool>("value");
    QTest::newRow("true") << true;
    QTest::newRow("false") << false;
}

void TestTreeNode::testInitInt() {
    SecreC::TreeNode::Location loc(1, 2, 3, 4, "testInitInt");

    QFETCH(uint64_t, value1);

    TreeNodeExprInt n(value1, loc);
    QCOMPARE(n.type(), NODE_LITE_INT);
    QCOMPARE(n.value(), value1);
}

void TestTreeNode::testInitInt_data() {
    QTest::addColumn<uint64_t>("value1");
    QTest::addColumn<uint64_t>("value2");

    NR2(-42ul,-42ul); NR2(-42ul,-1ul); NR2(-42ul,0ul); NR2(-42ul,1ul); NR2(-42ul,42ul);
    NR2(-1ul,-42ul);  NR2(-1ul,-1ul);  NR2(-1ul,0ul);  NR2(-1ul,1ul);  NR2(-1ul,42ul);
    NR2(0ul,-42ul);   NR2(0ul,-1ul);   NR2(0ul,0ul);   NR2(0ul,1ul);   NR2(0ul,42ul);
    NR2(1ul,-42ul);   NR2(1ul,-1ul);   NR2(1ul,0ul);   NR2(1ul,1ul);   NR2(1ul,42ul);
    NR2(42ul,-42ul);  NR2(42ul,-1ul);  NR2(42ul,0ul);  NR2(42ul,1ul);  NR2(42ul,42ul);
}

void TestTreeNode::testInitString() {
    SecreC::TreeNode::Location loc(1, 2, 3, 4, "testInitString");

    QFETCH(QString, value1);
    QFETCH(QString, value2);

    TreeNodeExprString n(value1.toStdString(), loc);
    QCOMPARE(n.type(), NODE_LITE_STRING);
    QCOMPARE(n.value().str(), value1.toStdString());
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
    typedef TreeNode::ChildrenList::size_type ST;
    SecreC::TreeNode::Location loc(1, 2, 3, 4, "testChildren");

    TreeNodeExprString *n  = new TreeNodeExprString("n", loc);
    TreeNodeExprString *c1 = new TreeNodeExprString("c1", loc);
    TreeNodeExprString *c2 = new TreeNodeExprString("c2", loc);
    QCOMPARE(n->children().size(), (ST) 0);
    n->appendChild(c1);
    QCOMPARE(n->children().size(), (ST) 1);
    QCOMPARE(n->children().at(0), c1);
    n->appendChild(c2);
    QCOMPARE(n->children().size(), (ST) 2);
    QCOMPARE(n->children().at(0), c1);
    QCOMPARE(n->children().at(1), c2);

    delete n;
}

QTEST_APPLESS_MAIN(TestTreeNode)
