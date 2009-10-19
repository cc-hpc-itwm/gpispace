# -*- mode: cmake; -*-
include(FindBoost)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES})

file(GLOB tc_srcs "test_*.cpp")
foreach (tc_src ${tc_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.cpp.*" "\\2" tc_name ${tc_src})
  message(STATUS "adding test case ${tc_name} (using ${tc_src})")

  add_executable(${tc_name} ${tc_src})
  target_link_libraries(${tc_name} fhglog)
  target_link_libraries(${tc_name} fhglog_remote)
  get_target_property(LOC ${tc_name} LOCATION_)
  add_test(${tc_name} ${LOC})
endforeach(tc_src ${tc_srcs})

