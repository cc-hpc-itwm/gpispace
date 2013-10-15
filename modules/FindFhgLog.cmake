# -*- mode: cmake; -*-
# locates the FHG Logging framework
# This file defines:
# * FHGLOG_FOUND if fhglog was found
# * FHGLOG_LIBRARY The lib to link to (currently only a static unix lib)
# * FHGLOG_INCLUDE_DIR

if (NOT FhgLog_FIND_QUIETLY)
  message(STATUS "FindFhgLog check")
endif (NOT FhgLog_FIND_QUIETLY)

if (NOT TARGET fhglog)
  #  include(FindPackageHelper)
  #  check_package(FhgLog fhglog/fhglog.hpp fhglog 1.0)

  find_path (FhgLog_INCLUDE_DIR
	NAMES "fhglog/fhglog.hpp"
	HINTS ${FHGLOG_HOME} ENV FHGLOG_HOME
	PATH_SUFFIXES include
  )

  find_library (FhgLog_LIBRARY
	NAMES libfhglog.a
	HINTS ${FHGLOG_HOME} ENV FHGLOG_HOME
	PATH_SUFFIXES lib
  )
  find_library (FhgLog_LIBRARY_SHARED
	NAMES libfhglog.so libfhglog.dylib
	HINTS ${FHGLOG_HOME} ENV FHGLOG_HOME
	PATH_SUFFIXES lib
  )

  if (FhgLog_INCLUDE_DIR AND FhgLog_LIBRARY)
	set (FhgLog_FOUND TRUE)
	if (NOT FhgLog_FIND_QUIETLY)
	  message (STATUS "Found FhgLog headers in ${FhgLog_INCLUDE_DIR} and libraries ${FhgLog_LIBRARY} ${FhgLog_LIBRARY_SHARED}")
	endif (NOT FhgLog_FIND_QUIETLY)
  else (FhgLog_INCLUDE_DIR AND FhgLog_LIBRARY)
	if (FhgLog_FIND_REQUIRED)
	  message (FATAL_ERROR "FhgLog could not be found!")
	endif (FhgLog_FIND_REQUIRED)
  endif (FhgLog_INCLUDE_DIR AND FhgLog_LIBRARY)

else (NOT TARGET fhglog)
  set(FhgLog_FOUND true)
  set(FhgLog_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/util ${CMAKE_SOURCE_DIR}/fhglog ${CMAKE_BINARY_DIR}/fhglog)
  set(FhgLog_LIBRARY_DIR "")
  set(FhgLog_LIBRARY fhglog)
  set(FhgLog_LIBRARY_SHARED fhglog-shared)
endif (NOT TARGET fhglog)
