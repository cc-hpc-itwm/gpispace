# -*- mode: cmake; -*-

if (WITH_REMOTE_LOGGING)
  add_executable(fhglog-server fhglog-server.cpp)
  add_executable(fhglog-client fhglog-client.cpp)

  target_link_libraries(fhglog-server fhglog_remote fhglog ${Boost_LIBRARIES})
  target_link_libraries(fhglog-client fhglog_remote fhglog ${Boost_LIBRARIES})

  install(TARGETS fhglog-server DESTINATION bin)
  install(TARGETS fhglog-client DESTINATION bin)
endif (WITH_REMOTE_LOGGING)
