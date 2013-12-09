# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${FVM_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/gspc)
include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

include_directories(${UTIL_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

add_library(concurrent_mod SHARED concurrent.cpp)
set_target_properties(concurrent_mod PROPERTIES OUTPUT_NAME concurrent)

install(TARGETS concurrent_mod
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules)
