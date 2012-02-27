set (PARSU_DIFF "${SDPA_APPS_DIR}/parsu/diff")

add_subdirectory (xml)

set (PARSU_DIFF_DIR "\${SDPA_HOME}/${PARSU_DIFF}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/share/sdpa/xml/lib")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_diff.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_diff
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_diff
)

install(PROGRAMS ${prgs}
  DESTINATION ${SDPA_APPS_DIR}/parsu/bin
  COMPONENT applications)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_DIFF}
  COMPONENT applications)
