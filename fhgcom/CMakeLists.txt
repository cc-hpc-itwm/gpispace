project (libfhgcom)

set (FHG_COM_NS_UUID "c9fe00cb-d9f7-432e-9235-66b7929b6e2a" CACHE STRING "The unique namespace uuid for this installation")

include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${${PROJECT_NAME}_SOURCE_DIR})

include (CheckCompiler)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif()

include (FindThreads)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT})
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (ZLIB REQUIRED QUIET)
include_directories (SYSTEM ${ZLIB_INCLUDE_DIRS})

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/fhgcom/ns_uuid.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/fhgcom/ns_uuid.cpp)

link_directories (${CMAKE_LIBRARY_PATH})
link_directories (${${PROJECT_NAME}_BINARY_DIR})

add_subdirectory (fhgcom)
add_subdirectory (bin)

if (BUILD_TESTING)
  include (CTest)
  add_subdirectory (tests)
endif()
