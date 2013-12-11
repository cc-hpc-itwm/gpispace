# mirko.rahn@itwm.fraunhofer.de

add_library (wfhd SHARED wfhd.c)
set_target_properties (wfhd PROPERTIES OUTPUT_NAME wfhd VERSION 1)

install (TARGETS wfhd
  LIBRARY DESTINATION lib
  COMPONENT runtime
  )
install (FILES
  wfhd.h
  DESTINATION include/we/util
  COMPONENT runtime-headers
)
