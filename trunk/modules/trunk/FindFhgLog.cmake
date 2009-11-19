# -*- mode: cmake; -*-
# locates the FHG Logging framework
# This file defines:
# * FHGLOG_FOUND if fhglog was found
# * FHGLOG_LIBRARY The lib to link to (currently only a static unix lib) 
# * FHGLOG_INCLUDE_DIR

message(STATUS "FindFhglog check")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FindPackageHelper)
  check_package(FHGLOG fhglog/fhglog.hpp fhglog 1.0)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(FHGLOG_FOUND true)
  set(FHGLOG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/fhglog ${CMAKE_BINARY_DIR}/fhglog)
  set(FHGLOG_LIBRARY_DIR "")
  set(FHGLOG_LIBRARY fhglog)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
