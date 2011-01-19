
set (PARSU_STACK "share/sdpa/parsu/stack")

add_subdirectory (xml)

set (PARSU_STACK_DIR "${CMAKE_INSTALL_PREFIX}/${PARSU_STACK}")
set (PARSU_PNETC_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${PNETC_LIB_DIR}")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_stack.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_stack
                @ONLY
               )
 
set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_stack
)

install(PROGRAMS ${prgs}
  DESTINATION bin/parsu
  COMPONENT parsu)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION ${PARSU_STACK}
  COMPONENT parsu)
