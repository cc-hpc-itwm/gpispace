add_executable(fhgmodinfo fhg-modinfo.cpp)
target_link_libraries(fhgmodinfo fhg-plugin)

install (TARGETS fhgmodinfo
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
