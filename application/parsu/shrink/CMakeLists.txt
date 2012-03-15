
set (PARSU_SHRINK "${SDPA_APPS_DIR}/parsu/shrink")

add_subdirectory (xml)

set (PARSU_SHRINK_DIR "\${SDPA_HOME}/${PARSU_SHRINK}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/share/sdpa/xml/lib")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_shrink.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_shrink
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_shrink
)

install(PROGRAMS ${prgs}
  DESTINATION ${SDPA_APPS_DIR}/parsu/bin
  COMPONENT applications)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_SHRINK}
  COMPONENT applications)

