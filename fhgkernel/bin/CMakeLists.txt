add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

add_executable(fhgkernel fhgkernel.cpp)
target_link_libraries(fhgkernel fhg-plugin we-dev ${UTIL_LIBRARIES})
if (NOT ENABLE_LICENSE_CHECK)
  set_target_properties(fhgkernel PROPERTIES COMPILE_DEFINITIONS "FHG_DISABLE_LICENSE_CHECK")
endif()

install (TARGETS fhgmodinfo fhgkernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
