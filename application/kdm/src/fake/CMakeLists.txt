include_directories(${FVM_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

include_directories(${UTIL_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

add_library(kdm_mod_fake SHARED kdm_complex_mod.cpp)
set_target_properties(kdm_mod_fake PROPERTIES OUTPUT_NAME kdm_complex)

install(TARGETS kdm_mod_fake
  LIBRARY
  DESTINATION libexec/fake
  COMPONENT modules
  )
