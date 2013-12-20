# -*- mode: cmake; -*-

add_executable(fhglogd server.cpp)
add_executable(fhglogc client.cpp)
add_executable(fhglog-dump dump.cpp)

target_link_libraries(fhglogd fhglog ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries(fhglogc fhglog ${Boost_PROGRAM_OPTIONS_LIBRARY})
target_link_libraries(fhglog-dump fhglog ${Boost_PROGRAM_OPTIONS_LIBRARY})

install(TARGETS fhglogd fhglogc fhglog-dump
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)
