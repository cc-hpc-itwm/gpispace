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
            fhglog
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

fhg_add_runtime_executable (NAME pnetv
  SOURCES "pnetv.cpp"
  LIBRARIES fhg-revision
            pnetv-jpna
            pnetv-jpn
            pnet
            Util::Generic
            Boost::program_options
)

fhg_add_runtime_executable (NAME pnetd
  SOURCES "pnetd.cpp"
  LIBRARIES fhg-revision
            pnet
            gspc
            ${Boost_PROGRAM_OPTIONS_LIBRARY}
            ${Boost_SERIALIZATION_LIBRARY}
            ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (NAME fhglog-dump
  SOURCES "fhglog-dump.cpp"
  LIBRARIES fhglog
            Boost::program_options
            Util::Generic
)
