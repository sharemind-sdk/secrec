include(../../../config.pri)
CONFIG += qt qtestlib

TARGET = ../../../bin/test-libscc-treenode
INCLUDEPATH += ../../../include
LIBS += -L../../../lib -lscc

SOURCES = \
    testtreenode.cpp
