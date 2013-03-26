include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

include(FindUTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src/we)

include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
  link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_subdirectory(xml)
add_subdirectory(xsd)
