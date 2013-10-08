add_executable (gspcmonc gspcmonc.cpp)
target_link_libraries (gspcmonc
  gspcmon_client
  ${QT_LIBRARIES} fhg_util_qt fhg-util
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

install (TARGETS gspcmonc
  DESTINATION bin
  COMPONENT runtime
)
