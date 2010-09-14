# -*- mode: cmake; -*-
# locates the FHG Communication framework
# This file defines:
# * FHGCOM_FOUND if fhglog was found
# * FHGCOM_LIBRARY The lib to link to (currently only a static unix lib)
# * FHGCOM_INCLUDE_DIR

if (NOT FhgCom_FIND_QUIETLY)
  message(STATUS "FindFhgCom check")
endif (NOT FhgCom_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (FhgCom_INCLUDE_DIR
	NAMES "fhgcom/fhgcom.hpp"
	HINTS ${FHGCOM_HOME} ENV FHGCOM_HOME
	PATH_SUFFIXES include
  )

  find_library (FhgCom_LIBRARY
	NAMES libfhgcom.a
	HINTS ${FHGLOG_HOME} ENV FHGLOG_HOME
	PATH_SUFFIXES lib
  )
  find_library (FhgCom_LIBRARY_SHARED
	NAMES libfhgcom.so
	HINTS ${FHGLOG_HOME} ENV FHGLOG_HOME
	PATH_SUFFIXES lib
  )

  if (FhgCom_INCLUDE_DIR AND FhgCom_LIBRARY)
	set (FhgCom_FOUND TRUE)
	if (NOT FhgCom_FIND_QUIETLY)
	  message (STATUS "Found FhgCom headers in ${FhgCom_INCLUDE_DIR} and libraries ${FhgCom_LIBRARY} ${FhgCom_LIBRARY_SHARED}")
	endif (NOT FhgCom_FIND_QUIETLY)
  else (FhgCom_INCLUDE_DIR AND FhgCom_LIBRARY)
	if (FhgCom_FIND_REQUIRED)
	  message (FATAL_ERROR "FhgCom could not be found!")
	endif (FhgCom_FIND_REQUIRED)
  endif (FhgCom_INCLUDE_DIR AND FhgCom_LIBRARY)

else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(FhgCom_FOUND true)
  set(FhgCom_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/fhgcom ${CMAKE_BINARY_DIR}/fhgcom)
  set(FhgCom_LIBRARY_DIR "")
  set(FhgCom_LIBRARY fhgcom)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
