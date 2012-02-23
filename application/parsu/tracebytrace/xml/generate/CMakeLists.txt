
set (xml
  package.xml
  slot.xml
)

install(FILES ${xml}
  DESTINATION ${PARSU_TRACEBYTRACE}/generate
  COMPONENT applications)
