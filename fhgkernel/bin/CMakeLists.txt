add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

add_executable(fhgkernel fhgkernel.cpp)
target_link_libraries(fhgkernel fhg-plugin we-dev ${UTIL_LIBRARIES})

install (TARGETS fhgmodinfo fhgkernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
