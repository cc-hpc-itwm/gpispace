add_executable (gspcnetd gspcnet-daemon.cpp)
target_link_libraries (gspcnetd
  gspcnet
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)
