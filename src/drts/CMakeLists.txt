# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (worker)

install (FILES
  drts.hpp
  virtual_memory.hpp
  DESTINATION include/drts/
)

add_library (gspc
  drts.cpp
  system.cpp
  virtual_memory.cpp
)
target_link_libraries (gspc
  ${Boost_FILESYSTEM_LIBRARY}
)

install (TARGETS
  gspc
  DESTINATION lib/
)
