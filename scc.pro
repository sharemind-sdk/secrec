include(scc.pri)

TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS = src tests

SCC_TESTS = \
    bin/test-libscc-parse \
    bin/test-libscc-treenode






for(t,SCC_TESTS){
  !win32 {
    runtests.commands += \
        LD_LIBRARY_PATH=lib/ "$$t" $$escape_expand(\n\t)
  }
  win32 {
    runtests.commands += \
        $$t $$escape_expand(\n\t)
  }
}
QMAKE_EXTRA_TARGETS += runtests
