set(ENV{PATH} "${PATH}")
file (MAKE_DIRECTORY "${OUTPUT}")
execute_process (
     COMMAND "${A2X}" -f "${FORMAT}" ${OPTION} --no-xmllint "--destination-dir=${OUTPUT}" "${INPUT}"
     RESULT_VARIABLE __res
)
if (NOT ${__res} EQUAL 0)
  file (REMOVE_RECURSE "${OUTPUT}")
endif ()
