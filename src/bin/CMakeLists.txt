fhg_add_runtime_executable (NAME pnet2dot
  SOURCES "pnet2dot.cpp"
  LIBRARIES fhg-revision
            pnet
            Util::Generic
            Boost::program_options
            Boost::serialization
            Boost::system
)

fhg_add_runtime_executable (NAME pnetc
  SOURCES "pnetc.cpp"
  LIBRARIES fhg-revision
            pnet
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
)

fhg_add_runtime_executable (NAME agent
  SOURCES "agent.cpp"
  LIBRARIES sdpa
            pnet
            Util::Generic
            rif-started_process_promise
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)
