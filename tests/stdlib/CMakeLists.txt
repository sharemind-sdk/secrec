#
# This will generate runtests.sh with the correct paths if
# SHAREMIND_PATH is set in config.local
#

IF(DEFINED SHAREMIND_PATH)

FILE(READ runtests_template SCRIPT)

SET(HEAD "#!/bin/sh
# WARNING: this script is generated automatically from runtests_template.

SECREC=${CMAKE_SOURCE_DIR}
SHAREMIND=${SHAREMIND_PATH}
")

SET(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/tests/stdlib/runtests.sh")

EXECUTE_PROCESS(COMMAND "sh" "-c" "echo \"${HEAD} ${SCRIPT}\" > ${SCRIPT_PATH}; chmod u+x ${SCRIPT_PATH}")

ENDIF()
