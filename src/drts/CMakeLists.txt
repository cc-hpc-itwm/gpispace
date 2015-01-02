# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (worker)

install (FILES
  client.fwd.hpp
  client.hpp
  drts.fwd.hpp
  drts.hpp
  stream.fwd.hpp
  stream.hpp
  information_to_reattach.fwd.hpp
  information_to_reattach.hpp
  virtual_memory.fwd.hpp
  virtual_memory.hpp
  DESTINATION include/drts/
)

add_library (gspc SHARED
  client.cpp
  drts.cpp
  information_to_reattach.cpp
  private/option.cpp
  private/startup_and_shutdown.cpp
  stream.cpp
  virtual_memory.cpp
)
target_link_libraries (gspc
  sdpa
  rif
  pnet
  gpi-space-pc-client-static
  gpi-space-pc-segment
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_REGEX_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install (TARGETS
  gspc
  DESTINATION lib/
)

add_subdirectory (test/parallel_running_workflows)
add_subdirectory (test/put_token)
