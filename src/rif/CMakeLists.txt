find_package (Libssh2 REQUIRED)

gspc_add_library (NAME rif-entry_point
  TYPE STATIC
  SOURCES "entry_point.cpp"
  LIBRARIES PRIVATE Util::Generic
            PRIVATE Boost::serialization
)

gspc_add_executable (NAME gspc-rifd
  RUNTIME
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES Util::RPC
            fhg-util
            gspc::logging
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
            PRIVATE rif-entry_point
  CREATE_BUNDLE_INFO
)

gspc_add_library (NAME rif-started_process_promise
  # Object library for sake of re-exporting in libgspc.
  TYPE OBJECT
  SOURCES "started_process_promise.cpp"
  LIBRARIES GPISpace::header-only
            Boost::iostreams
            Boost::program_options
            Boost::serialization
            Util-Generic
)

install (FILES
  started_process_promise.hpp
  started_process_promise.ipp
  DESTINATION include/rif
)

add_unit_test (NAME started_process_promise
  SOURCES "test/started_process_promise.cpp"

  DESCRIPTION "pipe used for communicating with a rif-started process during startup"
  USE_BOOST
  LIBRARIES rif-started_process_promise
            Util-Generic
)

gspc_add_library (NAME rif-strategies
  TYPE STATIC
  SOURCES "strategy/local.cpp"
          "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/ssh.cpp"
          "strategy/ssh/context.cpp"
          "strategy/ssh/detail.cpp"
          "strategy/ssh/session.cpp"
  LIBRARIES
    PRIVATE Util::RPC
    PRIVATE Util::Generic
    PRIVATE Libssh2::libssh2
    PRIVATE OpenSSL::Crypto
    PRIVATE OpenSSL::SSL
    PRIVATE rif-entry_point
    PUBLIC Boost::filesystem
    PUBLIC Boost::program_options
    PUBLIC Boost::serialization
    PRIVATE Boost::thread
)

gspc_add_executable (NAME gspc-bootstrap-rifd
  RUNTIME
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-project_configuration
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
gspc_add_executable (NAME gspc-teardown-rifd
  RUNTIME
  SOURCES "gspc-teardown-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-project_configuration
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
