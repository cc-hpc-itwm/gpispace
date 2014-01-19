include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${UTIL_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

set (hdrs
  process.hpp
)

add_library(process ${hdrs} process.cpp)
add_library(process.shared SHARED ${hdrs} process.cpp)
set_target_properties(process.shared PROPERTIES OUTPUT_NAME process)
target_link_libraries (process ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (process.shared ${Boost_FILESYSTEM_LIBRARY})

install(FILES ${hdrs}
  DESTINATION include/process
  COMPONENT module-headers
)

install(TARGETS process.shared
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules)

include (CTest)
  add_subdirectory (test)
