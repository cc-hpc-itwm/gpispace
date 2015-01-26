add_executable (orchestrator orchestrator.cpp)
target_link_libraries (orchestrator
  sdpa
  ${FhgLog_LIBRARY}
  pnet
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
install (TARGETS orchestrator
  RUNTIME
  DESTINATION bin
)

add_executable (agent agent.cpp)
target_link_libraries (agent
  sdpa
  ${FhgLog_LIBRARY}
  pnet
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
install (TARGETS agent
  RUNTIME
  DESTINATION bin
)

add_executable (sdpac sdpac.cpp)
target_link_libraries (sdpac
      sdpa
      fhg-revision
      ${FhgLog_LIBRARY}
      ${Boost_FILESYSTEM_LIBRARY}
      ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
install (TARGETS sdpac
  RUNTIME
  DESTINATION bin
)
