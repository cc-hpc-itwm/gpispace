
set (xml
  package.xml
  slot.xml
)

install(PROGRAMS ${xml}
  DESTINATION share/sdpa/parsu/shrink/generate
  COMPONENT parsu)
