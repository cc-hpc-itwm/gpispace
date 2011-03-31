set (daemon_libs
  sdpa-events
  sdpa-daemon
  sdpa-util
  sdpa-com
  sdpa
  seda
  ${WE_LIBRARIES}
  ${Boost_LIBRARIES}
)

#
# Orchestrator
#
add_executable(orchestrator orchestrator.cpp)
target_link_libraries(orchestrator ${daemon_libs})
install(TARGETS orchestrator
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

#
# Aggregator
#
add_executable(aggregator aggregator.cpp)
target_link_libraries(aggregator ${daemon_libs})
install(TARGETS aggregator
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
  
#
# Nre
#
add_executable(nre nre.cpp)
target_link_libraries(nre ${daemon_libs})
install(TARGETS nre
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

set ( client_libs
      sdpa-client
      sdpa-events
      sdpa-util
      sdpa-com
      sdpa
      seda
      ${Boost_LIBRARIES} 
)

#
# Client
#
add_executable(sdpac sdpac.cpp)
target_link_libraries(sdpac ${client_libs})
install(TARGETS sdpac
  RUNTIME
  DESTINATION bin
  COMPONENT applications)

add_executable(bsdpac blocking_client.cpp)
target_link_libraries(bsdpac ${client_libs})
install(TARGETS bsdpac
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
