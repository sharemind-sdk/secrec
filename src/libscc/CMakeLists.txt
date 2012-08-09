################################################################################
# libscc
################################################################################

BISON_TARGET(SECREC_PARSER "secrec.y"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")
FLEX_TARGET(SECREC_LEXER "secrec.l"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")
ADD_FLEX_BISON_DEPENDENCY(SECREC_LEXER SECREC_PARSER)

SET(LIBSCC_INCLUDE_DIR
    "${CMAKE_SOURCE_DIR}/include/libscc/")

SET(FLEX_BISON_SOURCES
    "secrec.l"
    "secrec.y")

SET(LIBSCC_SOURCES
    "blocks.cpp"
    "icodelist.cpp"
    "imop.cpp"
    "intermediate.cpp"
    "misc.cpp"
    "dataflowanalysis.cpp"
    "symboltable.cpp"
    "treenode.cpp"
    "virtual_machine.cpp"
    "constant.cpp"
    "symbol.cpp"
    "codegen/statement.cpp"
    "codegen/expression.cpp"
    "codegen/assign.cpp"
    "codegen/program.cpp"
    "codegen.cpp"
    "types.cpp"
    "typechecker.cpp"
    "typechecker/templates.h"
    "typechecker/templates.cpp"
    "typechecker/procedures.cpp"
    "typechecker/Expression.cpp"
    "typechecker/Statement.cpp"
    "TypeContext.cpp"
    "analysis/ReachingDeclassify.cpp"
    "analysis/ReachingDefinitions.cpp"
    "analysis/ReachingJumps.cpp"
    "analysis/ReachableReleases.cpp"
    "analysis/LiveVariables.cpp"
    "analysis/LiveMemory.cpp"
    "analysis/Dominators.cpp"
    "context_impl.h"
    "context_impl.cpp"
    "context.cpp"
    "ModuleInfo.h"
    "ModuleInfo.cpp"
    "ModuleMap.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")


SET(LIBSCC_HEADERS
    "${LIBSCC_INCLUDE_DIR}/CFG.h"
    "${LIBSCC_INCLUDE_DIR}/codegen.h"
    "${LIBSCC_INCLUDE_DIR}/dataflowanalysis.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDeclassify.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDefinitions.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingJumps.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachableReleases.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveVariables.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveMemory.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/Dominators.h"
    "${LIBSCC_INCLUDE_DIR}/imop.h"
    "${LIBSCC_INCLUDE_DIR}/log.h"
    "${LIBSCC_INCLUDE_DIR}/parser.h"
    "${LIBSCC_INCLUDE_DIR}/symbol.h"
    "${LIBSCC_INCLUDE_DIR}/treenode.h"
    "${LIBSCC_INCLUDE_DIR}/virtual_machine.h"
    "${LIBSCC_INCLUDE_DIR}/blocks.h"
    "${LIBSCC_INCLUDE_DIR}/codegenResult.h"
    "${LIBSCC_INCLUDE_DIR}/constant.h"
    "${LIBSCC_INCLUDE_DIR}/icodelist.h"
    "${LIBSCC_INCLUDE_DIR}/intermediate.h"
    "${LIBSCC_INCLUDE_DIR}/misc.h"
    "${LIBSCC_INCLUDE_DIR}/symboltable.h"
    "${LIBSCC_INCLUDE_DIR}/types.h"
    "${LIBSCC_INCLUDE_DIR}/typechecker.h"
    "${LIBSCC_INCLUDE_DIR}/TypeContext.h"
    "${LIBSCC_INCLUDE_DIR}/context.h"
    "${LIBSCC_INCLUDE_DIR}/ModuleMap.h")

SET_SOURCE_FILES_PROPERTIES("libscc/treenode.cpp" PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

include_directories(
    "${CMAKE_SOURCE_DIR}/include/libscc/"
    "${CMAKE_SOURCE_DIR}/src/libscc/"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Boost_INCLUDE_DIRS})

ADD_LIBRARY("libscc" SHARED
    ${LIBSCC_SOURCES}
    ${FLEX_BISON_SOURCES}
    ${LIBSCC_HEADERS})
SET_TARGET_PROPERTIES("libscc" PROPERTIES
    OUTPUT_NAME "scc"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    VERSION "0.3.0.0"
    SOVERSION "0.3"
    CLEAN_DIRECT_OUTPUT 1)
TARGET_LINK_LIBRARIES("libscc"
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY})
INSTALL(TARGETS "libscc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
