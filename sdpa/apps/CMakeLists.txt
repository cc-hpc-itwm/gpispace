# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (orchestrator.cpp
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_IOSTREAMS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (agent.cpp
  LINK_LIBRARIES sdpa
                 ${FhgLog_LIBRARY}
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_IOSTREAMS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (sdpac.cpp
  LINK_LIBRARIES sdpa
                 fhg-revision
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
