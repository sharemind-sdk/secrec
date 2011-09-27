#ifndef TEST_TREENODE_H
#define TEST_TREENODE_H

#include <QtTest>


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

#endif
