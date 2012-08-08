################################################################################
# sca
################################################################################

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/include/" ${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE("sca" "main.cpp")
SET_TARGET_PROPERTIES("sca" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
    VERSION "0.3.0.0")
TARGET_LINK_LIBRARIES("sca" "libscc"
    "${Boost_PROGRAM_OPTIONS_LIBRARY}")
INSTALL(TARGETS "sca"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    PUBLIC_HEADER DESTINATION "include")
