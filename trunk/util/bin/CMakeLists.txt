add_executable(fhgcfg fhgcfg.cpp)
target_link_libraries(fhgcfg ${Boost_LIBRARIES})

add_executable(fhg-modinfo fhg-modinfo.cpp)
target_link_libraries(fhg-modinfo fhg-plugin dl)

install (TARGETS fhgcfg
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
