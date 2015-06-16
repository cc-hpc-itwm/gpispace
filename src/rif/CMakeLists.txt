# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME gspc-rifd
  SOURCES "gspc-rifd.cpp"
          "execute_and_get_startup_messages.cpp"
  LIBRARIES RPC
            Network
            fhg-util
            fhglog
            Util::Generic
            Boost::filesystem
            Boost::iostreams
            Boost::program_options
            Boost::system
            Boost::thread
)

extended_add_library (NAME rif-started_process_promise
  SOURCES "started_process_promise.cpp"
  LIBRARIES Boost::iostreams
            Boost::program_options
)

extended_add_library (NAME rif-strategies
  SOURCES "strategy/meta.cpp"
          "strategy/ssh.cpp"
  LIBRARIES RPC
            Network
            fhg-util
)

fhg_add_runtime_executable (NAME gspc-bootstrap-rifd
  SOURCES "gspc-bootstrap-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-util
            Util::Generic
            Boost::filesystem
            Boost::program_options
            Boost::system
)
fhg_add_runtime_executable (NAME gspc-teardown-rifd
  SOURCES "gspc-teardown-rifd.cpp"
  LIBRARIES rif-strategies
            fhg-util
            Util::Generic
            Network
            Boost::filesystem
            Boost::program_options
            Boost::system
)
