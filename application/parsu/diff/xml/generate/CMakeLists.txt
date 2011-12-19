
set (xml
  package.xml
  slot.xml
)

install(FILES ${xml}
  DESTINATION ${PARSU_DIFF}/generate
  COMPONENT applications)
