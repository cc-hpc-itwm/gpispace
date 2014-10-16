# bernd.loerwald@itwm.fraunhofer.de

add_library (rif "execute_and_get_startup_messages.cpp")
target_link_libraries (rif
  fhg-util
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)
