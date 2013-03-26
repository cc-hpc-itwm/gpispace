# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

include (FindThreads)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT})
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

include (FindUTIL REQUIRED)
include_directories (${UTIL_INCLUDE_DIR})

add_subdirectory (apps)
add_subdirectory (we)
add_subdirectory (xml)
add_subdirectory (xsd)

if (BUILD_TESTING)
  add_subdirectory (tests)
endif()
