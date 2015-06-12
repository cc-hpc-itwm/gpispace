# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME orchestrator
  SOURCES "orchestrator.cpp"
  LIBRARIES sdpa
            fhglog
            pnet
            Util::Generic
            Network
            rif-startup-messages-pipe
            Boost::program_options
            Boost::system
)

fhg_add_runtime_executable (NAME agent
  SOURCES "agent.cpp"
  LIBRARIES sdpa
            fhglog
            pnet
            Util::Generic
            Network
            rif-startup-messages-pipe
            Boost::program_options
            Boost::system
)
