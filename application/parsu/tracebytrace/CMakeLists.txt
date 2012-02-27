
set (PARSU_TRACEBYTRACE "${SDPA_APPS_DIR}/parsu/tracebytrace")

add_subdirectory (xml)

set (PARSU_TRACEBYTRACE_DIR "\${SDPA_HOME}/${PARSU_TRACEBYTRACE}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/share/sdpa/xml/lib")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_tracebytrace.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_tracebytrace
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_tracebytrace
)

install(PROGRAMS ${prgs}
  DESTINATION ${SDPA_APPS_DIR}/parsu/bin
  COMPONENT applications)
