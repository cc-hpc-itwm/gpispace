
add_subdirectory (xml)

set (PARSU_SHRINK_DIR
    ${CMAKE_INSTALL_PREFIX}/share/sdpa/parsu/shrink
)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/parsu_shrink.in
                ${CMAKE_CURRENT_BINARY_DIR}/parsu_shrink
                @ONLY
               )
 
set (prgs
   ${CMAKE_CURRENT_BINARY_DIR}/parsu_shrink
)

install(PROGRAMS ${prgs}
  DESTINATION bin/parsu
  COMPONENT parsu)

set (helper
   mk.xml
)

install(PROGRAMS ${helper}
  DESTINATION share/sdpa/parsu/shrink
  COMPONENT parsu)

