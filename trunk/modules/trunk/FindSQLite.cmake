# - Try to find the Sqlite encryption library
# Once done this will define
#
#  SQLITE_FOUND - system has the Sqlite library
#  SQLITE_INCLUDE_DIR - the Sqlite include directory
#  SQLITE_LIBRARIES - The libraries needed to use Sqlite

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SQLITE_HOME MATCHES "")
  if("" MATCHES "$ENV{SQLITE_HOME}")
    message(STATUS "SQLITE_HOME env is not set, setting it to /usr/local")
    set (SQLITE_HOME "/usr/local")
  else("" MATCHES "$ENV{SQLITE_HOME}")
    set (SQLITE_HOME "$ENV{SQLITE_HOME}")
  endif("" MATCHES "$ENV{SQLITE_HOME}")
else(SQLITE_HOME MATCHES "")
  message(STATUS "SQLITE_HOME is not empty: \"${SQLITE_HOME}\"")
  set (SQLITE_HOME "${SQLITE_HOME}")
endif(SQLITE_HOME MATCHES "")

IF(SQLITE_LIBRARIES)
   SET(SQLITE_FIND_QUIETLY TRUE)
ENDIF(SQLITE_LIBRARIES)

FIND_PATH(SQLITE_INCLUDE_DIR sqlite3.h 
  ${SQLITE_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include)

message(STATUS "Inc ${SQLITE_INCLUDE_DIR}")

IF(WIN32 AND MSVC)
   # /MD and /MDd are the standard values - if somone wants to use
   # others, the libnames have to change here too

   IF(WIN32)
     SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
   ELSE(WIN32)
     SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
   ENDIF(WIN32)

   FIND_LIBRARY(SQLITE_DEBUG NAMES libsqlite3 PATH_SUFFIXES lib)
   FIND_LIBRARY(SQLITE_RELEASE NAMES libsqlite3 PATH_SUFFIXES lib)

   IF(MSVC_IDE)
      IF(SQLITE_DEBUG AND SQLITE_RELEASE)
         SET(SQLITE_LIBRARIES optimized ${SQLITE_RELEASE} debug ${SQLITE_DEBUG})
      ELSE(SQLITE_DEBUG AND SQLITE_RELEASE)
         MESSAGE(FATAL_ERROR "Could not find the debug and release version of sqlite")
      ENDIF(SQLITE_DEBUG AND SQLITE_RELEASE)
   ELSE(MSVC_IDE)
      STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
      IF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
         SET(SQLITE_LIBRARIES ${SQLITE_DEBUG})
      ELSE(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
         SET(SQLITE_LIBRARIES ${SQLITE_RELEASE})
      ENDIF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
   ENDIF(MSVC_IDE)
   MARK_AS_ADVANCED(SQLITE_DEBUG SQLITE_RELEASE)
ELSE(WIN32 AND MSVC)

   SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
   FIND_LIBRARY(SQLITE_LIBRARIES NAMES sqlite3 )

ENDIF(WIN32 AND MSVC)

# evaluate search results
IF(SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES)
   SET(SQLITE_FOUND TRUE)
ELSE(SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES)
   SET(SQLITE_FOUND FALSE)
ENDIF (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARIES)

IF (SQLITE_FOUND)
   IF (NOT Sqlite_FIND_QUIETLY)
      MESSAGE(STATUS "Found Sqlite: ${SQLITE_LIBRARIES}")
   ENDIF (NOT Sqlite_FIND_QUIETLY)
ELSE (SQLITE_FOUND)
   IF (Sqlite_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could NOT find Sqlite")
   ENDIF (Sqlite_FIND_REQUIRED)
ENDIF (SQLITE_FOUND)

MARK_AS_ADVANCED(SQLITE_INCLUDE_DIR SQLITE_LIBRARIES)

