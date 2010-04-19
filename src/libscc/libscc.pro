include(../../scc.pri)

INCLUDEPATH += ../../include/libscc/
TARGET = ../../lib/scc
TEMPLATE = lib

SOURCES = \
    blocks.cpp \
    icodelist.cpp \
    imop.cpp \
    intermediate.cpp \
    misc.cpp \
    symboltable.cpp \
    treenode.cpp \
    types.cpp

OTHER_FILES = \
    secrec.l \
    secrec.y

HEADERS = \
    ../../include/libscc/blocks.h \
    ../../include/libscc/icodelist.h \
    ../../include/libscc/imop.h \
    ../../include/libscc/intermediate.h \
    ../../include/libscc/log.h \
    ../../include/libscc/misc.h \
    ../../include/libscc/parser.h \
    ../../include/libscc/sccobject.h \
    ../../include/libscc/sccpointer.h \
    ../../include/libscc/symboltable.h \
    ../../include/libscc/treenode.h \
    ../../include/libscc/types.h

GENERATED_SOURCES += lex_secrec.c yacc_secrec.tab.c
GENERATED_FILES   += lex_secrec.h yacc_secrec.tab.h

SCC_LEX = flex
SCC_YACC = bison
SCC_LEX_FLAGS += -8
SCC_YACC_FLAGS +=

target_yacc_h.target = yacc_secrec.tab.h
target_yacc_h.depends = secrec.y ../../include/libscc/parser.h
target_yacc_h.commands = $${SCC_YACC} -d -b yacc_secrec $${SCC_YACC_FLAGS} secrec.y $$escape_expand(\n\t) \
                         $$QMAKE_MOVE yacc_secrec.tab.c yacc_secrec.tmp $$escape_expand(\n\t)
silent:target_yacc_h.commands = @echo Bison H ${QMAKE_FILE_IN} && $$target_yacc_h.commands

target_yacc_c.target = yacc_secrec.tab.c
target_yacc_c.depends = target_yacc_h target_lex_h
target_yacc_c.commands = $$QMAKE_COPY yacc_secrec.tmp yacc_secrec.tab.c $$escape_expand(\n)
silent:target_yacc_c.commands = @echo Bison C ${QMAKE_FILE_IN} && $$target_yacc_c.commands

target_lex_h.target = lex_secrec.h
target_lex_h.depends = secrec.l ../../include/libscc/parser.h
target_lex_h.commands = $${SCC_LEX} -o lex_secrec.tmp --header-file=lex_secrec.h $${SCC_LEX_FLAGS} secrec.l $$escape_expand(\n)
silent:target_lex_h.commands = @echo Flex H ${QMAKE_FILE_IN} && $$target_lex_h.commands

target_lex_c.target = lex_secrec.c
target_lex_c.depends = target_lex_h target_yacc_h
target_lex_c.commands = $$QMAKE_COPY lex_secrec.tmp lex_secrec.c $$escape_expand(\n)
silent:target_lex_c.commands = @echo Flex C ${QMAKE_FILE_IN} && $$target_lex_c.commands

clean_lex.commands  = $$QMAKE_DEL_FILE lex_secrec.tmp  lex_secrec.h  lex_secrec.c
silent:clean_lex.commands = @echo Cleaning flex files && $$clean_lex.commands

clean_yacc.commands = $$QMAKE_DEL_FILE yacc_secrec.tmp yacc_secrec.tab.h yacc_secrec.tab.c
silent:clean_yacc.commands = @echo Cleaning flex files && $$clean_yacc.commands

QMAKE_EXTRA_TARGETS += target_lex_c target_lex_h target_yacc_c target_yacc_h clean_lex clean_yacc

CLEAN_DEPS += clean_lex clean_yacc

