# -*- mode: cmake; -*-
# locates cppunit
# This file defines:
# * CppUnit_FOUND if libreadline was found
# * CppUnit_LIBRARY The lib to link to (currently only a static unix lib)
# * CppUnit_LIBRARY_SHARED The lib to link to (currently only a static unix lib)
# * CppUnit_INCLUDE_DIR

if (NOT CppUnit_FIND_QUIETLY)
  message(STATUS "FindCppUnit check")
endif (NOT CppUnit_FIND_QUIETLY)

find_path (CppUnit_INCLUDE_DIR
  NAMES "cppunit/TestSuite.h"
  HINTS ${CPPUNIT_HOME} ENV CPPUNIT_HOME
  PATH_SUFFIXES include
  )

if(WIN32)
  set(LIBRARY_STATIC_NAME "cppunit.lib")
  set(LIBRARY_SHARED_NAME "cppunit_dll.dll")
ELSE(WIN32)
  set(LIBRARY_STATIC_NAME "libcppunit.a")
  set(LIBRARY_SHARED_NAME "libcppunit.so")
ENDIF(WIN32)

find_library (CppUnit_LIBRARY
  NAMES ${LIBRARY_STATIC_NAME}
  HINTS ${CPPUNIT_HOME} ENV CPPUNIT_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (CppUnit_LIBRARY_SHARED
  NAMES ${LIBRARY_SHARED_NAME}
  HINTS ${CPPUNIT_HOME} ENV CPPUNIT_HOME
  PATH_SUFFIXES lib lib64
  )

if (NOT CppUnit_FIND_QUIETLY)
  message (STATUS "Found CppUnit headers in ${CppUnit_INCLUDE_DIR} and libraries ${CppUnit_LIBRARY} ${CppUnit_LIBRARY_SHARED}")
endif()
if (CppUnit_INCLUDE_DIR AND ( CppUnit_LIBRARY OR CppUnit_LIBRARY_SHARED))
  if( NOT CppUnit_LIBRARY)
    set(CppUnit_LIBRARY ${CppUnit_LIBRARY_SHARED})
  endif( NOT CppUnit_LIBRARY)

  set (CppUnit_FOUND TRUE)
  if (NOT CppUnit_FIND_QUIETLY)
    message (STATUS "Found CppUnit headers in ${CppUnit_INCLUDE_DIR} and libraries ${CppUnit_LIBRARY} ${CppUnit_LIBRARY_SHARED}")
  endif (NOT CppUnit_FIND_QUIETLY)
else (CppUnit_INCLUDE_DIR AND ( CppUnit_LIBRARY OR CppUnit_LIBRARY_SHARED))
  if (CppUnit_FIND_REQUIRED)
    message (FATAL_ERROR "CppUnit could not be found!")
  endif (CppUnit_FIND_REQUIRED)
endif (CppUnit_INCLUDE_DIR AND ( CppUnit_LIBRARY OR CppUnit_LIBRARY_SHARED))
