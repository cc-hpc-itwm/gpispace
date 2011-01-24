
add_subdirectory (generate)

set (xml
   op2.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_OPTWO}
  COMPONENT parsu)
