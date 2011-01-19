
add_subdirectory (generate)

set (xml
   shrink.xml.in
)

install(PROGRAMS ${xml}
  DESTINATION share/sdpa/parsu/shrink
  COMPONENT parsu)
