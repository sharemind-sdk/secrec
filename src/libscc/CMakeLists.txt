################################################################################
# libscc
################################################################################

find_package(Boost COMPONENTS thread program_options filesystem system REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(SECREC_PARSER "secrec.y"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")
FLEX_TARGET(SECREC_LEXER "secrec.l"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")
ADD_FLEX_BISON_DEPENDENCY(SECREC_LEXER SECREC_PARSER)

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
    "types.cpp"
    "virtual_machine.cpp"
    "constant.cpp"
    "symbol.cpp"
    "codegen/statement.cpp"
    "codegen/expression.cpp"
    "codegen/assign.cpp"
    "codegen/program.cpp"
    "codegen.cpp"
    "typechecker.cpp"
    "typechecker/templates.h"
    "typechecker/templates.cpp"
    "typechecker/procedures.cpp"
    "context_impl.h"
    "context_impl.cpp"
    "context.cpp"
    "ModuleInfo.h"
    "ModuleInfo.cpp"
    "ModuleMap.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")

SET(LIBSCC_HEADERS
    "${CMAKE_SOURCE_DIR}/include/libscc/codegen.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/dataflowanalysis.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/imop.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/log.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/parser.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/symbol.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/treenode.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/virtual_machine.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/blocks.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/codegenResult.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/constant.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/icodelist.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/intermediate.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/misc.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/symboltable.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/types.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/typechecker.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/context.h"
    "${CMAKE_SOURCE_DIR}/include/libscc/ModuleMap.h")

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
TARGET_LINK_LIBRARIES("libscc" "${Boost_SYSTEM_LIBRARY}" "${Boost_FILESYSTEM_LIBRARY}")
INSTALL(TARGETS "libscc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
