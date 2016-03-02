#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

FIND_PACKAGE(SharemindCHeaders REQUIRED)
ADD_DEFINITIONS(${SharemindCHeaders_DEFINITIONS})

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
    "analysis/ConstantFolding.cpp"
    "analysis/Dominators.cpp"
    "analysis/LiveMemory.cpp"
    "analysis/LiveVariables.cpp"
    "analysis/ReachableReleases.cpp"
    "analysis/ReachingDeclassify.cpp"
    "analysis/ReachingDefinitions.cpp"
    "analysis/ReachingJumps.cpp"
    "APInt.cpp"
    "APFloat.cpp"
    "Blocks.cpp"
    "CastTemplateChecker.cpp"
    "CastTypeUnifier.cpp"
    "CodeGen.cpp"
    "CodeGenResult.cpp"
    "codegen/Assign.cpp"
    "codegen/Expression.cpp"
    "codegen/LValue.cpp"
    "codegen/Program.cpp"
    "codegen/Statement.cpp"
    "Constant.cpp"
    "Context.cpp"
    "ContextImpl.cpp"
    "ContextImpl.h"
    "DataflowAnalysis.cpp"
    "DataType.cpp"
    "ICodeList.cpp"
    "Imop.cpp"
    "Intermediate.cpp"
    "Location.cpp"
    "Log.cpp"
    "Misc.cpp"
    "ModuleInfo.cpp"
    "ModuleInfo.h"
    "ModuleMap.cpp"
    "OperatorTemplateChecker.cpp"
    "OperatorTypeUnifier.cpp"
    "optimizer/ConstantFolding.cpp"
    "optimizer/CopyElimination.cpp"
    "Optimizer.cpp"
    "optimizer/DeadVariableElimination.cpp"
    "optimizer/RemoveUnreachableBlocks.cpp"
    "SecurityType.cpp"
    "StringRef.cpp"
    "Symbol.cpp"
    "SymbolTable.cpp"
    "TemplateChecker.cpp"
    "TreeNode.cpp"
    "TypeArgument.cpp"
    "TypeChecker.cpp"
    "typechecker/Expression.cpp"
    "typechecker/LValue.cpp"
    "typechecker/procedures.cpp"
    "typechecker/Statement.cpp"
    "typechecker/Struct.cpp"
    "typechecker/Templates.cpp"
    "typechecker/Templates.h"
    "typechecker/Type.cpp"
    "TypeContext.cpp"
    "TypeUnifier.cpp"
    "Types.cpp"
    "VirtualMachine.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c"
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h")


SET(LIBSCC_HEADERS
    "${LIBSCC_INCLUDE_DIR}/analysis/ConstantFolding.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/Dominators.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveMemory.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/LiveVariables.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachableReleases.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDeclassify.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingDefinitions.h"
    "${LIBSCC_INCLUDE_DIR}/analysis/ReachingJumps.h"
    "${LIBSCC_INCLUDE_DIR}/APInt.h"
    "${LIBSCC_INCLUDE_DIR}/APFloat.h"
    "${LIBSCC_INCLUDE_DIR}/Blocks.h"
    "${LIBSCC_INCLUDE_DIR}/CFG.h"
    "${LIBSCC_INCLUDE_DIR}/CodeGen.h"
    "${LIBSCC_INCLUDE_DIR}/CodeGenResult.h"
    "${LIBSCC_INCLUDE_DIR}/CodeGenState.h"
    "${LIBSCC_INCLUDE_DIR}/Constant.h"
    "${LIBSCC_INCLUDE_DIR}/Context.h"
    "${LIBSCC_INCLUDE_DIR}/DataflowAnalysis.h"
    "${LIBSCC_INCLUDE_DIR}/DataType.h"
    "${LIBSCC_INCLUDE_DIR}/ICodeList.h"
    "${LIBSCC_INCLUDE_DIR}/Imop.h"
    "${LIBSCC_INCLUDE_DIR}/Intermediate.h"
    "${LIBSCC_INCLUDE_DIR}/Location.h"
    "${LIBSCC_INCLUDE_DIR}/Log.h"
    "${LIBSCC_INCLUDE_DIR}/Misc.h"
    "${LIBSCC_INCLUDE_DIR}/ModuleMap.h"
    "${LIBSCC_INCLUDE_DIR}/Optimizer.h"
    "${LIBSCC_INCLUDE_DIR}/ParserEnums.h"
    "${LIBSCC_INCLUDE_DIR}/Parser.h"
    "${LIBSCC_INCLUDE_DIR}/PrettyPrint.h"
    "${LIBSCC_INCLUDE_DIR}/SecurityType.h"
    "${LIBSCC_INCLUDE_DIR}/StringRef.h"
    "${LIBSCC_INCLUDE_DIR}/StringTable.h"
    "${LIBSCC_INCLUDE_DIR}/SymbolFwd.h"
    "${LIBSCC_INCLUDE_DIR}/Symbol.h"
    "${LIBSCC_INCLUDE_DIR}/SymbolTable.h"
    "${LIBSCC_INCLUDE_DIR}/TemplateChecker.h"
    "${LIBSCC_INCLUDE_DIR}/TreeNodeC.h"
    "${LIBSCC_INCLUDE_DIR}/TreeNodeFwd.h"
    "${LIBSCC_INCLUDE_DIR}/TreeNode.h"
    "${LIBSCC_INCLUDE_DIR}/TypeArgument.h"
    "${LIBSCC_INCLUDE_DIR}/TypeChecker.h"
    "${LIBSCC_INCLUDE_DIR}/TypeContext.h"
    "${LIBSCC_INCLUDE_DIR}/Types.h"
    "${LIBSCC_INCLUDE_DIR}/TypeUnifier.h"
    "${LIBSCC_INCLUDE_DIR}/VirtualMachine.h"
    "${LIBSCC_INCLUDE_DIR}/Visitor.h")

SET_SOURCE_FILES_PROPERTIES("libscc/treenode.cpp" PROPERTIES OBJECT_DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/lex_secrec.h;${CMAKE_CURRENT_BINARY_DIR}/yacc_secrec.c")

include_directories(
    "${CMAKE_SOURCE_DIR}/include/libscc/"
    "${CMAKE_SOURCE_DIR}/src/libscc/"
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${MPFR_INCLUDE_DIRS}
    ${SharemindCHeaders_INCLUDE_DIRS})

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
    ${MPFR_LIBRARIES}
    ${SharemindCHeaders_LIBRARIES})
INSTALL(TARGETS "libscc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
