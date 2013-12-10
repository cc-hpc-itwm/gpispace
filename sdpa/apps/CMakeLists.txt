set (daemon_libs
  sdpa-daemon
  sdpa-com
  sdpa
  ${FhgLog_LIBRARY}
  ${SEDA_LIBRARY}
  pnet
  ${Boost_PROGRAM_OPTIONS_LIBRARY} # orchestrator, agent
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
  COMPONENT runtime)

#
# Agent
#

add_executable(agent agent.cpp)
target_link_libraries(agent ${daemon_libs})
install(TARGETS agent
  RUNTIME
  DESTINATION bin
  COMPONENT runtime)


set ( client_libs
      sdpa-client
      sdpa-com
      sdpa
      fhg-revision
      ${FhgLog_LIBRARY}
      ${SEDA_LIBRARY}
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_PROGRAM_OPTIONS_LIBRARY}
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
  COMPONENT runtime)
