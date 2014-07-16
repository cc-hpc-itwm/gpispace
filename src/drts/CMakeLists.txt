# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (worker)

install (FILES
  drts.hpp
  system.hpp
  vmem.hpp
  DESTINATION include/drts/
)

add_library (gspc
  drts.cpp
  system.cpp
  vmem.cpp
)
target_link_libraries (gspc
  ${Boost_FILESYSTEM_LIBRARY}
)

install (TARGETS
  gspc
  DESTINATION lib/
)
