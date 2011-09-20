set (daemon_libs
  sdpa-events
  sdpa-daemon
  sdpa-com
  sdpa
#  sdpa-util
  ${FhgLog_LIBRARY}
  ${SEDA_LIBRARY}
  ${WE_LIBRARIES}
  ${Boost_LIBRARIES}
)

# FIXME: I get a linker error, when I try to link against sdpa-util (Bad value),
# which seems to be  a known compiler bug in gcc 4.1.2,  by explicitly using the
# source files, everything is fine but that just sucks.
set(util-sources
  ${CMAKE_SOURCE_DIR}/sdpa/sdpa/util/Config.cpp
  ${CMAKE_SOURCE_DIR}/sdpa/sdpa/util/util.cpp
)

#
# Orchestrator
#
add_executable(orchestrator orchestrator.cpp ${util-sources})
target_link_libraries(orchestrator ${daemon_libs})
install(TARGETS orchestrator
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

#
# Aggregator
#
#add_executable(aggregator aggregator.cpp)
#target_link_libraries(aggregator ${daemon_libs})
#install(TARGETS aggregator
#  RUNTIME
#  DESTINATION bin
#  COMPONENT applications)

#
# Agent
#

add_executable(agent agent.cpp ${util-sources})
target_link_libraries(agent ${daemon_libs})
install(TARGETS agent
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

#
# Nre
#
#add_executable(nre nre.cpp)
#target_link_libraries(nre ${daemon_libs})
#install(TARGETS nre
#  RUNTIME
#  DESTINATION bin
#  COMPONENT applications)


set ( client_libs
      sdpa-client
      sdpa-events
#      sdpa-util
      sdpa-com
      sdpa
      ${FhgLog_LIBRARY}
      ${SEDA_LIBRARY}
      ${Boost_LIBRARIES}
)

#
# Client
#
add_executable(sdpac sdpac.cpp ${util-sources})
target_link_libraries(sdpac ${client_libs})
install(TARGETS sdpac
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

#add_executable(bsdpac blocking_client.cpp)
#target_link_libraries(bsdpac ${client_libs})
#install(TARGETS bsdpac
#  RUNTIME
#  DESTINATION bin
#  COMPONENT applications)

#add_executable(suca submit_and_cancel.cpp)
#target_link_libraries(suca ${client_libs})
