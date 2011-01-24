
set (PARSU_DIFF "share/sdpa/parsu/diff")

add_subdirectory (xml)

set (PARSU_DIFF_DIR "${CMAKE_INSTALL_PREFIX}/${PARSU_DIFF}")
set (PARSU_PNETC_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${PNETC_LIB_DIR}")

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
  DESTINATION ${PARSU_DIFF}
  COMPONENT parsu)
