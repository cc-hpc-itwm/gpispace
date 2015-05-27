# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable ("gspc-rifd.cpp" "execute_and_get_startup_messages.cpp"
  LINK_LIBRARIES RPC
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

extended_add_library (NAME rif-startup-messages-pipe
  SOURCES "startup_messages_pipe.cpp"
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

fhg_add_runtime_executable ("gspc-bootstrap-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 Util::Generic
                 Boost::filesystem
                 Boost::program_options
                 Boost::system
)
fhg_add_runtime_executable ("gspc-teardown-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 Util::Generic
                 Network
                 Boost::filesystem
                 Boost::program_options
                 Boost::system
)
