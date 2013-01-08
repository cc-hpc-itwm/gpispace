# -*- mode: cmake; -*-

function (add_we_sources)
  foreach (source ${ARGN})
    set (WE_SOURCES ${WE_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/${source}
      CACHE INTERNAL "we_sources" FORCE
    )
  endforeach()
endfunction()

#! \todo eliminate this (will nev
set (WE_NUM_EXTRACTORS "1" CACHE STRING "number of extractors")
set (WE_NUM_INJECTORS  "1" CACHE STRING "number of injectors")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp
  DESTINATION include/we
  COMPONENT headers)
add_definitions(-DHAVE_CONFIG_H)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(SDPAFindBoost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

include(FindUTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(we)
add_subdirectory(apps)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)
