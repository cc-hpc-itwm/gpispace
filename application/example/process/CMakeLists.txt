# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${FVM_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/gspc)
include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

include_directories(${UTIL_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

add_library(exec_mod SHARED process.cpp exec_mod.cpp)
set_target_properties(exec_mod PROPERTIES OUTPUT_NAME exec)

set (hdrs
  process.hpp
)

add_library(process ${hdrs} process.cpp)
add_library(process.shared SHARED ${hdrs} process.cpp)
set_target_properties(process.shared PROPERTIES OUTPUT_NAME process)
target_link_libraries (process ${Boost_FILESYSTEM_LIBRARY})
target_link_libraries (process.shared ${Boost_FILESYSTEM_SHARED_LIBRARY})

set (xmls
  exec.xml
  )

install(FILES ${xmls}
  DESTINATION share/sdpa/xml/process
  COMPONENT examples)

install(FILES ${hdrs}
  DESTINATION include/process
  COMPONENT module-headers
)

install(TARGETS process.shared
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules)

include (CTest)
if (BUILD_TESTING)
  add_subdirectory (test)
endif (BUILD_TESTING)
