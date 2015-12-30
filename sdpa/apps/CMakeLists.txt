# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME orchestrator
  SOURCES "orchestrator.cpp"
  LIBRARIES sdpa
            fhglog
            pnet
            Util::Generic
            Network
            rif-started_process_promise
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

fhg_add_runtime_executable (NAME agent
  SOURCES "agent.cpp"
  LIBRARIES sdpa
            fhglog
            pnet
            Util::Generic
            Network
            rif-started_process_promise
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)
