
add_subdirectory (generate)

set (xml
   diff.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_DIFF}
  COMPONENT parsu)
