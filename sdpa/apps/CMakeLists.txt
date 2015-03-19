# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (orchestrator.cpp
  LINK_LIBRARIES sdpa
                 fhglog
                 pnet
                 Util::Generic
                 Network
                 rif-startup-messages-pipe
                 Boost::program_options
                 Boost::system
)

fhg_add_runtime_executable (agent.cpp
  LINK_LIBRARIES sdpa
                 fhglog
                 pnet
                 Util::Generic
                 Network
                 rif-startup-messages-pipe
                 Boost::program_options
                 Boost::system
)
