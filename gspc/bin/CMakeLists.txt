add_executable (gspcnetd gspcnet-daemon.cpp)
target_link_libraries (gspcnetd
  libgspc
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (gspcnetc gspcnet-client.cpp)
target_link_libraries (gspcnetc
  libgspc
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (gspcnet-ping gspcnet-ping.cpp)
target_link_libraries (gspcnet-ping
  libgspc
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

add_executable (gspcnet-req gspcnet-req.cpp)
target_link_libraries (gspcnet-req
  libgspc
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

install (TARGETS gspcnetd gspcnetc gspcnet-ping gspcnet-req
  DESTINATION bin
  COMPONENT runtime
)
add_executable (gspc gspc.cpp)
target_link_libraries (gspc
  libgspc
  ${UTIL_LIBRARIES}
  ${FhgLog_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY} # fhglog
  ${Boost_SYSTEM_LIBRARY}
)

# TODO: activate when full replacement for 'gspc' script is working
#install (TARGETS gspc
#  DESTINATION bin
#  COMPONENT runtime
#)
