
set (xml
  package.xml
  slot.xml
)

install(FILES ${xml}
  DESTINATION ${PARSU_SHRINK}/generate
  COMPONENT applications)
