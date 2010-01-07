include(config.pri)

TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS = src tests

runtests_debug.commands = \
    LD_LIBRARY_PATH=lib/ bin/test-libscc-treenode$$escape_expand(\n\t) \
    LD_LIBRARY_PATH=lib/ bin/test-libscc-parse$$escape_expand(\n\t)
runtests_debug.depends = debug

runtests_release.commands = \
    LD_LIBRARY_PATH=lib/ bin/test-libscc-treenode$$escape_expand(\n\t) \
    LD_LIBRARY_PATH=lib/ bin/test-libscc-parse$$escape_expand(\n\t)
runtests_release.depends = release

runtests.depends = runtests_debug runtests_release

QMAKE_EXTRA_TARGETS += runtests runtests_debug runtests_release
