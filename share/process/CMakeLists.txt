include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${UTIL_INCLUDE_DIR})

add_library (process process.cpp)
add_library (process.shared SHARED process.cpp)
set_target_properties (process.shared PROPERTIES OUTPUT_NAME process)
target_link_libraries (process
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
)
target_link_libraries (process.shared
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
)

install (FILES process.hpp
  DESTINATION include/process
  COMPONENT module-headers
)

install (TARGETS process.shared
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules
)

add_subdirectory (test)
