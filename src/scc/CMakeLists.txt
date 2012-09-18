################################################################################
# scc
################################################################################

SET(SCC_SOURCES
    "main.cpp"
    "VMCode.h"
    "VMCode.cpp"
    "Compiler.h"
    "Compiler.cpp"
    "CopyElimination.h"
    "CopyElimination.cpp"
    "DeadVariableElimination.h"
    "DeadVariableElimination.cpp"
    "Builtin.h"
    "Builtin.cpp"
    "SyscallManager.h"
    "SyscallManager.cpp"
    "RegisterAllocator.h"
    "RegisterAllocator.cpp"
    "RemoveUnreachableBlocks.h"
    "RemoveUnreachableBlocks.cpp"
    "ScalarAllocPlacement.h"
    "ScalarAllocPlacement.cpp"
    "StringLiterals.h"
    "StringLiterals.cpp"
    "VMInstruction.h"
    "VMInstruction.cpp"
    "VMSymbolTable.h"
    "VMSymbolTable.cpp"
    "VMValue.h"
    "VMValue.cpp"
    "VMDataType.h")

include_directories("${CMAKE_SOURCE_DIR}/include/" ${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE("scc" ${SCC_SOURCES})
SET_TARGET_PROPERTIES("scc" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.0.1.0")
TARGET_LINK_LIBRARIES("scc" "libscc" "libas"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    )
INSTALL(TARGETS "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
