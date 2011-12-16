
set (PARSU_STACK "libexec/apps/parsu/stack")

add_subdirectory (xml)

set (PARSU_STACK_DIR "\${SDPA_HOME}/${PARSU_STACK}")
set (PARSU_PNETC_LIB_DIR "\${SDPA_HOME}/share/sdpa/xml/lib")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_stack.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_stack
                @ONLY
               )

set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_stack
)

install(PROGRAMS ${prgs}
  DESTINATION libexec/apps/parsu/bin
  COMPONENT applications)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_STACK}
  COMPONENT applications)
