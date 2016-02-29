# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME gspc-rifd
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES RPC
            fhg-util
            fhglog
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
  CREATE_BUNDLE_INFO
)

extended_add_library (NAME rif-started_process_promise
  SOURCES "started_process_promise.cpp"
  LIBRARIES Boost::iostreams
            Boost::program_options
            Boost::serialization
            Util-Generic
  INSTALL
)

install (FILES
  started_process_promise.hpp
  DESTINATION include/rif
)

add_unit_test (NAME started_process_promise
  SOURCES "test/started_process_promise.cpp"

  DESCRIPTION "pipe used for communicating with a rif-started process during startup"
  USE_BOOST
  LIBRARIES rif-started_process_promise
            Util-Generic
)

extended_add_library (NAME rif-strategies
  SOURCES "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/ssh.cpp"
  LIBRARIES RPC
            fhg-util
            libssh2_wrapper
)

fhg_add_runtime_executable (NAME gspc-bootstrap-rifd
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-revision
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
fhg_add_runtime_executable (NAME gspc-teardown-rifd
  SOURCES "gspc-teardown-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-revision
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
