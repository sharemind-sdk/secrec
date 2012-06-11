PROJECT(SHAREMIND_SCC "CXX")

IF(NOT SHAREMIND_EXT_VM_GLOBALS_DIR)
    SET(SHAREMIND_EXT_VM_GLOBALS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/vm_globals")
ENDIF(NOT SHAREMIND_EXT_VM_GLOBALS_DIR)

IF(NOT SHAREMIND_EXT_VM_M4_DIR)
    SET(SHAREMIND_EXT_VM_M4_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/vm_m4")
ENDIF(NOT SHAREMIND_EXT_VM_M4_DIR)

IF(NOT SHAREMIND_EXT_LIBEXECUTABLE_DIR)
    SET(SHAREMIND_EXT_LIBEXECUTABLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/libexecutable")
ENDIF(NOT SHAREMIND_EXT_LIBEXECUTABLE_DIR)

IF(NOT SHAREMIND_EXT_LIBVMI_DIR)
    SET(SHAREMIND_EXT_LIBVMI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/libvmi")
ENDIF(NOT SHAREMIND_EXT_LIBVMI_DIR)

IF(NOT SHAREMIND_EXT_LIBAS_DIR)
    SET(SHAREMIND_EXT_LIBAS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/libas")
ENDIF(NOT SHAREMIND_EXT_LIBAS_DIR)
IF(NOT EXISTS "${SHAREMIND_EXT_LIBAS_DIR}/src/sharemind/CMakeLists.txt")
    MESSAGE(FATAL "SHAREMIND_EXT_LIBAS_DIR is invalid!")
ENDIF()

IF(NOT SHAREMIND_LIBAS_SOURCE_DIR)
    ADD_SUBDIRECTORY("${SHAREMIND_EXT_LIBAS_DIR}/src/sharemind" "${SHAREMIND_EXT_DIR}/libas")
ENDIF(NOT SHAREMIND_LIBAS_SOURCE_DIR)


################################################################################
# scc
################################################################################

find_package(Boost COMPONENTS iostreams REQUIRED)

SET(SCC_SOURCES
    "main.cpp"
    "VMCode.h"
    "VMCode.cpp"
    "Compiler.h"
    "Compiler.cpp"
    "CopyElimination.h"
    "CopyElimination.cpp"
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

include_directories("${CMAKE_SOURCE_DIR}/include/"
                    "${SHAREMIND_EXT_LIBAS_DIR}/src"
                    "${SHAREMIND_EXT_LIBEXECUTABLE_DIR}/src"
                    "${SHAREMIND_EXT_LIBVMI_DIR}/src"
                    "${SHAREMIND_EXT_VM_GLOBALS_DIR}/include"
                    ${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE("scc" ${SCC_SOURCES})
SET_TARGET_PROPERTIES("scc" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.0.1.0")
TARGET_LINK_LIBRARIES("scc" "libscc" "libas"
    "${Boost_PROGRAM_OPTIONS_LIBRARY}"
    "${Boost_SYSTEM_LIBRARY}"
    "${Boost_FILESYSTEM_LIBRARY}"
    "${Boost_IOSTREAMS_LIBRARY}"
    )
INSTALL(TARGETS "scc"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
