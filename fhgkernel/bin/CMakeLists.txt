add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

add_executable(fhgkernel fhgkernel.cpp)
target_link_libraries(fhgkernel fhg-plugin)

install (TARGETS fhgmodinfo fhgkernel
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
