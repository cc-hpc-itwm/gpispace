add_executable(fhgcfg fhgcfg.cpp)
target_link_libraries(fhgcfg ${Boost_REGEX_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

install (TARGETS fhgcfg
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
