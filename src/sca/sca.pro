include(../../config.pri)

TARGET = ../../bin/sca

INCLUDEPATH += ../../include/
LIBS += -L../../lib/ -lscc

SOURCES = \
    main.c
