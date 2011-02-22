
set (PARSU_OPTWO "share/sdpa/parsu/op2")

add_subdirectory (xml)

set (PARSU_OPTWO_DIR "\${SDPA_HOME}/${PARSU_OPTWO}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/${PNETC_LIB_DIR}")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_op2.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_op2
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_op2
)

install(PROGRAMS ${prgs}
  DESTINATION bin/parsu
  COMPONENT parsu)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_OPTWO}
  COMPONENT parsu)
