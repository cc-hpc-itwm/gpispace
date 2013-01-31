# -*- mode: cmake; -*-

if (ENABLE_REMOTE_LOGGING)
  add_executable(fhglogd fhglog-server.cpp)
  add_executable(fhglogc fhglog-client.cpp)
  add_executable(fhglog-dump fhglog-dump.cpp)

  target_link_libraries(fhglogd fhglog ${Boost_LIBRARIES})
  target_link_libraries(fhglogc fhglog ${Boost_LIBRARIES})
  target_link_libraries(fhglog-dump fhglog ${Boost_LIBRARIES})

  install(TARGETS fhglogd fhglogc fhglog-dump
    RUNTIME
    DESTINATION bin
    COMPONENT runtime)
endif (ENABLE_REMOTE_LOGGING)
