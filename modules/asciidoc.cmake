set(ENV{PATH} "${PATH}")
execute_process (
     COMMAND ${ASCIIDOC} -b ${BACKEND} -o ${OUTPUT} ${INPUT}
     RESULT_VARIABLE __res
)
if (NOT ${__res} EQUAL 0)
  file (REMOVE_RECURSE "${OUTPUT}")
endif ()
