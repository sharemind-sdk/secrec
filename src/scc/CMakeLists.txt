################################################################################
# scc
################################################################################

SET(SCC_SOURCES
    "main.cpp"
    "VMCode.h"
    "VMCode.cpp"
    "Compiler.h"
    "Compiler.cpp"
    "Builtin.h"
    "Builtin.cpp"
    "SyscallManager.h"
    "SyscallManager.cpp"
    "RegisterAllocator.h"
    "RegisterAllocator.cpp"
    "StringLiterals.h"
    "StringLiterals.cpp"
    "VMInstruction.h"
    "VMInstruction.cpp"
    "VMSymbolTable.h"
    "VMSymbolTable.cpp"
    "VMValue.h"
    "VMValue.cpp"
    "VMDataType.h")

include_directories("${CMAKE_SOURCE_DIR}/include/")
ADD_EXECUTABLE("scc" ${SCC_SOURCES})
SET_TARGET_PROPERTIES("scc" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.0.1.0")
TARGET_LINK_LIBRARIES("scc" "libscc"
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY})
INSTALL(TARGETS "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
