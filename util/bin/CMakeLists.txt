add_executable(fhgcfg fhgcfg.cpp)
target_link_libraries(fhgcfg ${Boost_LIBRARIES} ${Boost_REGEX_LIBRARY})

install (TARGETS fhgcfg
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
