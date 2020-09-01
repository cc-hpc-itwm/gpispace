find_package (libssh2 REQUIRED COMPONENTS _relocatable)

fhg_add_runtime_executable (NAME iml-rifd
  SOURCES "iml-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES RPC
            IML::fhg-util
            IML::vmem-client
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
  CREATE_BUNDLE_INFO
)

extended_add_library (NAME rif-started_process_promise
  NAMESPACE IML
  SOURCES "started_process_promise.cpp"
  LIBRARIES Boost::iostreams
            Boost::program_options
            Boost::serialization
            Util-Generic
)

add_unit_test (NAME iml-started_process_promise
  SOURCES "test/started_process_promise.cpp"

  DESCRIPTION "pipe used for communicating with a iml-rif-started process during startup"
  USE_BOOST
  LIBRARIES IML::rif-started_process_promise
            Util-Generic
)

extended_add_library (NAME rif-strategies
  NAMESPACE IML
  SOURCES "strategy/local.cpp"
          "strategy/meta.cpp"
          "strategy/pbsdsh.cpp"
          "strategy/ssh.cpp"
          "strategy/ssh/context.cpp"
          "strategy/ssh/detail.cpp"
          "strategy/ssh/session.cpp"
  LIBRARIES RPC
            IML::fhg-util
            libssh2::libssh2
)

fhg_add_runtime_executable (NAME iml-bootstrap-rifd
  SOURCES "iml-bootstrap-rifd.cpp"
  LIBRARIES IML::rif-strategies
            IML::fhg-revision
            IML::fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
fhg_add_runtime_executable (NAME iml-teardown-rifd
  SOURCES "iml-teardown-rifd.cpp"
  LIBRARIES IML::rif-strategies
            IML::fhg-revision
            IML::fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
  CREATE_BUNDLE_INFO
)
