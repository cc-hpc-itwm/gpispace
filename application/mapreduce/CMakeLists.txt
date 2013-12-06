include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

include_directories(${FVM_INCLUDE_DIR})

include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${PROCESS_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(xml)
add_subdirectory (test)
