add_executable (gspcnetd gspcnet-daemon.cpp)
target_link_libraries (gspcnetd
  gspcnet
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (gspcnetc gspcnet-client.cpp)
target_link_libraries (gspcnetc
  gspcnet
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (gspcnet-ping gspcnet-ping.cpp)
target_link_libraries (gspcnet-ping
  gspcnet
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

install (TARGETS gspcnetd gspcnetc gspcnet-ping
  DESTINATION bin
  COMPONENT runtime
)
