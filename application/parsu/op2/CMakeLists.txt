
set (PARSU_OPTWO "libexec/apps/parsu/op2")

add_subdirectory (xml)

set (PARSU_OPTWO_DIR "\${SDPA_HOME}/${PARSU_OPTWO}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/share/sdpa/xml/lib")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_op2.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_op2
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_op2
)

install(PROGRAMS ${prgs}
  DESTINATION libexec/apps/parsu/bin
  COMPONENT applications)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_OPTWO}
  COMPONENT applications)
