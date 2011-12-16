
add_subdirectory (generate)

set (xml
   stack.xml.in
)

install(FILES ${xml}
  DESTINATION ${PARSU_STACK}
  COMPONENT applications)
