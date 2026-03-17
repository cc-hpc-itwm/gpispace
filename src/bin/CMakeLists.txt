gspc_add_executable (NAME pnet2dot
  RUNTIME
  SOURCES "pnet2dot.cpp"
  LIBRARIES gspc_configuration
            gspc_xml_parse
            gspc_util
            Boost::program_options
            Boost::serialization
            Boost::system
)

gspc_add_executable (NAME pnetc
  RUNTIME
  SOURCES "pnetc.cpp"
  LIBRARIES gspc_configuration
            gspc_xml_parse
            gspc_util
            Boost::program_options
            Boost::serialization
            Boost::system
)

gspc_add_executable (NAME gspc-agent
  RUNTIME
  # \note Exporting symbols so that modules see e.g. API guard symbol
  # and don't need to link explicitly against pnet or we.
  ENABLE_EXPORTS
  SOURCES "agent.cpp"
  LIBRARIES gspc_scheduler
            gspc_xml_parse
            gspc_util
            gspc_rif_started_process_promise
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

if (GSPC_WITH_MONITOR_APP)
  gspc_add_executable (NAME gspc-monitor
    RUNTIME
    SOURCES "gspc-monitor.cpp"
    LIBRARIES gspc_monitor
              Boost::program_options
    CREATE_BUNDLE_INFO
  )
endif()
