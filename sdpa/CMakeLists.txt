include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_SOURCE_DIR}) # sdpa/plugins/
include_directories (${CMAKE_SOURCE_DIR}/gspc)
include_directories (${CMAKE_SOURCE_DIR}/pnet/src)

include_directories (${UTIL_INCLUDE_DIR})

find_package (FhgKernel REQUIRED QUIET)
include_directories (${FhgKernel_INCLUDE_DIR})

find_package (SEDA REQUIRED QUIET)
include_directories (${SEDA_INCLUDE_DIR})

find_package(ZLIB REQUIRED QUIET)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})

find_package(GSPC REQUIRED QUIET)
include_directories(${GSPC_INCLUDE_DIR})

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

include_directories (${FhgLog_INCLUDE_DIR})

find_package (FhgCom REQUIRED QUIET)
include_directories (${FhgCom_INCLUDE_DIR})

include_directories (${MMGR_INCLUDE_DIR})

add_subdirectory (sdpa)
add_subdirectory (apps)
add_subdirectory (plugins)
add_subdirectory (bin)
add_subdirectory (etc)

if (BUILD_TESTING)
  add_subdirectory (tests)
endif (BUILD_TESTING)
