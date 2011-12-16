
add_subdirectory (generate)

set (xml
   shrink.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_SHRINK}
  COMPONENT applications)
