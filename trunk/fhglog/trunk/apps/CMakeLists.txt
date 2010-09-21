# -*- mode: cmake; -*-

if (WITH_REMOTE_LOGGING)
  add_executable(fhglogd fhglog-server.cpp)
  add_executable(fhglog-client fhglog-client.cpp)

  target_link_libraries(fhglogd fhglog ${Boost_LIBRARIES})
  target_link_libraries(fhglog-client fhglog ${Boost_LIBRARIES})

  install(TARGETS fhglogd
    RUNTIME
    DESTINATION bin
    COMPONENT applications)
endif (WITH_REMOTE_LOGGING)
