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

add_library (rif-startup-messages-pipe "startup_messages_pipe.cpp")
target_link_libraries (rif-startup-messages-pipe
  ${Boost_IOSTREAMS_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

add_library (rif-strategies "strategy/meta.cpp" "strategy/ssh.cpp")
target_link_libraries (rif-strategies RPC Network fhg-util)

fhg_add_runtime_executable ("gspc-bootstrap-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 Util::Generic
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
fhg_add_runtime_executable ("gspc-teardown-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 Util::Generic
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
