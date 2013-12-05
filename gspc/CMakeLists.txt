if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

include (SDPAFindBoost)
find_package(Threads REQUIRED QUIET)
find_package(UTIL REQUIRED QUIET)

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

include_directories (${CMAKE_SOURCE_DIR}/pnet/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(gspc)
add_subdirectory(bin)
add_subdirectory(libexec)
add_subdirectory(scripts)
add_subdirectory(doc)

add_subdirectory(tests)
