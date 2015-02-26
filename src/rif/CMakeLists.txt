# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable ("gspc-rifd.cpp" "execute_and_get_startup_messages.cpp"
  LINK_LIBRARIES rpc
                 net
                 fhg-util
                 fhglog
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_IOSTREAMS_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

add_library (rif-startup-messages-pipe "startup_messages_pipe.cpp")
target_link_libraries (rif-startup-messages-pipe
  ${Boost_IOSTREAMS_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

add_library (rif-strategies
  "strategy/meta.cpp"
  "strategy/poe.cpp"
  "strategy/ssh.cpp"
)
target_link_libraries (rif-strategies rpc net fhg-util)

fhg_add_runtime_executable ("gspc-bootstrap-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
fhg_add_runtime_executable ("gspc-teardown-rifd.cpp"
  LINK_LIBRARIES rif-strategies
                 fhg-util
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
