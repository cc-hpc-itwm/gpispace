add_library (process process.cpp)
add_library (process.shared SHARED process.cpp)
target_include_directories (process PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories (process.shared
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
set_target_properties (process.shared PROPERTIES OUTPUT_NAME process)
target_link_libraries (process
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  fhg-util
)
target_link_libraries (process.shared
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  fhg-util
)

install (FILES process.hpp
  DESTINATION include/process
)

install (TARGETS process.shared
  LIBRARY
  DESTINATION libexec/sdpa
)

add_subdirectory (test)
