
set (xml
  package.xml
  slot.xml
)

install(FILES ${xml}
  DESTINATION ${PARSU_OPTWO}/generate
  COMPONENT applications)
