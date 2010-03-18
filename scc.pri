CONFIG -= qt
CONFIG += debug

QMAKE_CLEAN=*~
QMAKE_CFLAGS_DEBUG=-std=c99 -DDEBUG -O0 -ggdb -Wall -Wextra -pedantic -pedantic-errors -Wno-long-long -pipe $$(CFLAGS)
QMAKE_CFLAGS_RELEASE=-std=c99 -DNDEBUG -O2 -pipe $$(CFLAGS)
QMAKE_CXXFLAGS_DEBUG=-std=c++98 -DDEBUG -O0 -ggdb -Wall -Wextra -pedantic -pedantic-errors -Wno-long-long -pipe $$(CXXFLAGS)
QMAKE_CXXFLAGS_RELEASE=-std=c++98 -DNDEBUG -O2 -pipe $$(CXXFLAGS)

DEFINES += _POSIX_SOURCE

win32:DEFINES += WIN32
unix:DEFINES  += UNIX _GNU_SOURCE
macx:DEFINES  += MACX

PRIVATECONFIG = config.pri
exists($$PRIVATECONFIG) {
    include($$PRIVATECONFIG)
}
