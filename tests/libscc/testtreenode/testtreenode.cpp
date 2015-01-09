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

#include "testtreenode.h"

#include <libscc/parser.h>
#include <libscc/treenode.h>


using namespace SecreC;

#define NR2(a,b) QTest::newRow(#a","#b)<<(a)<<(b)
#define NR3(a,b,c) QTest::newRow(#a","#b","#c)<<(a)<<(b)<<(c)
#define NR4(a,b,c,d) QTest::newRow(#a","#b","#c","#d)<<(a)<<(b)<<(c)<<(d)

void TestTreeNode::testInitTreeNode() {
    SecreC::Location loc(1, 2, 3, 4, "testInitTreeNode");
    SecreC::Location loc2(loc);

    SecreC::Location nloc(4, 3, 2, 1, "testInitTreeNode");
    SecreC::Location nloc2(nloc);

    TreeNode *n = new TreeNodeInternalUse(loc);
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
    SecreC::Location loc(1, 2, 3, 4, "testInitBool");

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
    SecreC::Location loc(1, 2, 3, 4, "testInitInt");

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
    /// \todo
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
    /// \todo
}

QTEST_APPLESS_MAIN(TestTreeNode)
