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

find_library (CppUnit_LIBRARY
  NAMES libcppunit.a
  HINTS ${CPPUNIT_HOME} ENV CPPUNIT_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (CppUnit_LIBRARY_SHARED
  NAMES libcppunit.so
  HINTS ${CPPUNIT_HOME} ENV CPPUNIT_HOME
  PATH_SUFFIXES lib lib64
  )

if (CppUnit_INCLUDE_DIR AND CppUnit_LIBRARY)
  set (CppUnit_FOUND TRUE)
  if (NOT CppUnit_FIND_QUIETLY)
    message (STATUS "Found CppUnit headers in ${CppUnit_INCLUDE_DIR} and libraries ${CppUnit_LIBRARY} ${CppUnit_LIBRARY_SHARED}")
  endif (NOT CppUnit_FIND_QUIETLY)
else (CppUnit_INCLUDE_DIR AND CppUnit_LIBRARY)
  if (CppUnit_FIND_REQUIRED)
    message (FATAL_ERROR "CppUnit could not be found!")
  endif (CppUnit_FIND_REQUIRED)
endif (CppUnit_INCLUDE_DIR AND CppUnit_LIBRARY)
