# -*- mode: cmake; -*-

if (WITH_REMOTE_LOGGING)
  add_executable(fhglogd fhglog-server.cpp)
  add_executable(fhglogc fhglog-client.cpp)

  target_link_libraries(fhglogd fhglog ${Boost_LIBRARIES})
  target_link_libraries(fhglogc fhglog ${Boost_LIBRARIES})

  install(TARGETS fhglogd fhglogc
    RUNTIME
    DESTINATION bin
    COMPONENT runtime)
endif (WITH_REMOTE_LOGGING)
