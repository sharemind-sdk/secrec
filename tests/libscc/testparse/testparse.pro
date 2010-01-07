include(../../../config.pri)
CONFIG += qt qtestlib

TARGET = ../../../bin/test-libscc-parse
INCLUDEPATH += ../../../include
LIBS += -L../../../lib -lscc

SOURCES = \
    testparse.cpp
