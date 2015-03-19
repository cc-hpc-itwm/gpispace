# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (orchestrator.cpp
  LINK_LIBRARIES sdpa
                 fhglog
                 pnet
                 Util::Generic
                 rif-startup-messages-pipe
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (agent.cpp
  LINK_LIBRARIES sdpa
                 fhglog
                 pnet
                 Util::Generic
                 rif-startup-messages-pipe
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
