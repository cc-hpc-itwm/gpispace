# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME gpi-space
  SOURCES "gpi-space.cpp"
  LIBRARIES gpi-space-gpiapi
            gpi-space-pc-container
            gpi-space-pc-global
            gpi-space-pc-memory
            gpi-space-pc-segment
            fhg-revision
            fhg-util
            mmgr
            fhgcom
            rif-started_process_promise
            Util::Generic
            ${CMAKE_THREAD_LIBS_INIT}
            Boost::filesystem
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

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
