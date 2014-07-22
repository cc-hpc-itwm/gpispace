# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (worker)

install (FILES
  drts.hpp
  virtual_memory.hpp
  DESTINATION include/drts/
)

add_library (gspc SHARED
  drts.cpp
  virtual_memory.cpp
)
target_link_libraries (gspc
  sdpa
  pnet
  gpi-space-pc-client-static
  gpi-space-pc-segment
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install (TARGETS
  gspc
  DESTINATION lib/
)
