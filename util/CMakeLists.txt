project (libfhgutil)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

include (CheckCompiler)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif()

find_package (Threads REQUIRED QUIET)
if (CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

add_subdirectory (fhg)
add_subdirectory (bin)

if (BUILD_TESTING)
  include (CTest)
  add_subdirectory (tests)
endif (BUILD_TESTING)
