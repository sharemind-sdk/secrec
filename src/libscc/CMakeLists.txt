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
    "analysis/Dominators.cpp"
    "analysis/LiveMemory.cpp"
    "analysis/LiveVariables.cpp"
    "analysis/ReachableReleases.cpp"
    "analysis/ReachingDeclassify.cpp"
    "analysis/ReachingDefinitions.cpp"
    "analysis/ReachingJumps.cpp"
    "blocks.cpp"
    "codegen.cpp"
    "CodeGenResult.cpp"
    "codegen/assign.cpp"
    "codegen/expression.cpp"
    "codegen/LValue.cpp"
    "codegen/program.cpp"
    "codegen/statement.cpp"
    "constant.cpp"
    "context.cpp"
    "context_impl.cpp"
    "context_impl.h"
    "dataflowanalysis.cpp"
    "DataType.cpp"
    "icodelist.cpp"
    "imop.cpp"
    "intermediate.cpp"
    "Location.cpp"
    "log.cpp"
    "misc.cpp"
    "ModuleInfo.cpp"
    "ModuleInfo.h"
    "ModuleMap.cpp"
    "SecurityType.cpp"
    "StringRef.cpp"
    "symbol.cpp"
    "symboltable.cpp"
    "treenode.cpp"
    "typechecker.cpp"
    "typechecker/Expression.cpp"
    "typechecker/LValue.cpp"
    "typechecker/procedures.cpp"
    "typechecker/Statement.cpp"
    "typechecker/Struct.cpp"
    "typechecker/templates.cpp"
    "typechecker/templates.h"
    "typechecker/Type.cpp"
    "TypeContext.cpp"
    "types.cpp"
    "virtual_machine.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")


SET(LIBSCC_HEADERS
    "${LIBSCC_INCLUDE_DIR}/analysis/Dominators.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveMemory.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveVariables.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachableReleases.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDeclassify.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDefinitions.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingJumps.h"
    "${LIBSCC_INCLUDE_DIR}/blocks.h"
    "${LIBSCC_INCLUDE_DIR}/CFG.h"
    "${LIBSCC_INCLUDE_DIR}/codegen.h"
    "${LIBSCC_INCLUDE_DIR}/CodeGenResult.h"
    "${LIBSCC_INCLUDE_DIR}/CodeGenState.h"
    "${LIBSCC_INCLUDE_DIR}/constant.h"
    "${LIBSCC_INCLUDE_DIR}/context.h"
    "${LIBSCC_INCLUDE_DIR}/dataflowanalysis.h"
    "${LIBSCC_INCLUDE_DIR}/DataType.h"
    "${LIBSCC_INCLUDE_DIR}/icodelist.h"
    "${LIBSCC_INCLUDE_DIR}/imop.h"
    "${LIBSCC_INCLUDE_DIR}/intermediate.h"
    "${LIBSCC_INCLUDE_DIR}/Location.h"
    "${LIBSCC_INCLUDE_DIR}/log.h"
    "${LIBSCC_INCLUDE_DIR}/misc.h"
    "${LIBSCC_INCLUDE_DIR}/ModuleMap.h"
    "${LIBSCC_INCLUDE_DIR}/parser.h"
    "${LIBSCC_INCLUDE_DIR}/SecurityType.h"
    "${LIBSCC_INCLUDE_DIR}/StringTable.h"
    "${LIBSCC_INCLUDE_DIR}/StringRef.h"
    "${LIBSCC_INCLUDE_DIR}/symbol.h"
    "${LIBSCC_INCLUDE_DIR}/symbol_fwd.h"
    "${LIBSCC_INCLUDE_DIR}/symboltable.h"
    "${LIBSCC_INCLUDE_DIR}/treenode.h"
    "${LIBSCC_INCLUDE_DIR}/treenode_c.h"
    "${LIBSCC_INCLUDE_DIR}/treenode_fwd.h"
    "${LIBSCC_INCLUDE_DIR}/typechecker.h"
    "${LIBSCC_INCLUDE_DIR}/TypeContext.h"
    "${LIBSCC_INCLUDE_DIR}/types.h"
    "${LIBSCC_INCLUDE_DIR}/virtual_machine.h")

SET_SOURCE_FILES_PROPERTIES("libscc/treenode.cpp" PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

include_directories(
    "${CMAKE_SOURCE_DIR}/include/libscc/"
    "${CMAKE_SOURCE_DIR}/src/libscc/"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS})

IF(NOT SHAREMIND_LIBSCC_LINKING_MODE)
    SET(SHAREMIND_LIBSCC_LINKING_MODE SHARED)
ENDIF()

ADD_LIBRARY("libscc" ${SHAREMIND_LIBSCC_LINKING_MODE}
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
    ${Boost_FILESYSTEM_LIBRARY}
    ${MPFR_LIBRARIES})
INSTALL(TARGETS "libscc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
