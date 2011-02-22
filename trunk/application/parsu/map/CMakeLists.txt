
set (PARSU_MAP "share/sdpa/parsu/map")

set (PARSU_MAP_DIR "\${SDPA_HOME}/${PARSU_MAP}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/${PNETC_LIB_DIR}")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_map.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_map
                @ONLY
               )

set (prgs
    ${CMAKE_CURRENT_BINARY_DIR}/parsu_map
)

install(PROGRAMS ${prgs}
  DESTINATION bin/parsu
  COMPONENT parsu)

set (xml
  finalize.xml
  init.xml
  loop.xml.in
  types.xml
  work.xml
)

set (conf
  tune.conf
)

install (PROGRAMS ${xml} ${conf}
  DESTINATION ${PARSU_MAP}
  COMPONENT parsu
)
