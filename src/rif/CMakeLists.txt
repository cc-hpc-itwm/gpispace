# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable ("gspc-rifd.cpp" "execute_and_get_startup_messages.cpp"
  LINK_LIBRARIES rpc
                 net
                 fhg-util
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_IOSTREAMS_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable ("gspc-bootstrap-rifd.cpp"
  LINK_LIBRARIES rpc
                 net
                 fhg-util
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)
