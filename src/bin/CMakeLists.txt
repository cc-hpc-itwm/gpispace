gspc_add_executable (NAME pnet2dot
  RUNTIME
  SOURCES "pnet2dot.cpp"
  LIBRARIES fhg-project_configuration
            pnet
            Util::Generic
            Boost::program_options
            Boost::serialization
            Boost::system
)

gspc_add_executable (NAME pnetc
  RUNTIME
  SOURCES "pnetc.cpp"
  LIBRARIES fhg-project_configuration
            pnet
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::system
)

gspc_add_executable (NAME agent
  RUNTIME
  # \note Exporting symbols so that modules see e.g. API guard symbol
  # and don't need to link explicitly against pnet or we.
  ENABLE_EXPORTS
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
