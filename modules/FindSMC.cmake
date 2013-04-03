# -*- mode: cmake; -*-
# Locate the SMC (State Machine Compiler)
# Smc can be found at http://smc.sourceforge.net/
# Written by Alexander Petry, petry _at_ itwm.fhg.de

# This module defines
# SMC_FOUND if the smc jar file could be found
# SMC_INCLUDE_DIR 
# SMC_JAR, where is the smc jar file
#
# Use the SMC_HOME environment variable to define the installation directory of SMC

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.6.2)
   message(FATAL_ERROR "CMake >= 2.6.2 required")
endif("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.6.2)
cmake_policy(PUSH)
cmake_policy(VERSION 2.6.2)

#cmake_minimum_required(VERSION "2.6.2" FATAL_ERROR)
Include(FindJava)

# set defaults
SET(_smc_HOME "/usr/local")
SET(_smc_INCLUDE_SEARCH_DIRS
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

SET(_smc_BINARY_SEARCH_DIRS
  ${CMAKE_BINARY_PATH}
  /usr/local/lib
  /usr/lib
  )

##
if( "${SMC_HOME}" STREQUAL "")
  if("" MATCHES "$ENV{SMC_HOME}")
    set (SMC_HOME ${_smc_HOME})
  else("" MATCHES "$ENV{SMC_HOME}")
    set (SMC_HOME "$ENV{SMC_HOME}")
  endif("" MATCHES "$ENV{SMC_HOME}")
else( "${SMC_HOME}" STREQUAL "")
endif( "${SMC_HOME}" STREQUAL "")
##

if (NOT SMC_FIND_QUIETLY)
  message(STATUS "Looking for SMC in ${SMC_HOME}")
endif()

IF( NOT ${SMC_HOME} STREQUAL "" )
    SET(_smc_INCLUDE_SEARCH_DIRS ${SMC_HOME}/lib/C++ ${_smc_INCLUDE_SEARCH_DIRS})
    SET(_smc_HOME ${SMC_HOME})
ENDIF( NOT ${SMC_HOME} STREQUAL "" )

IF( NOT $ENV{SMC_INCLUDEDIR} STREQUAL "" )
  SET(_smc_INCLUDE_SEARCH_DIRS $ENV{SMC_INCLUDEDIR} ${_smc_INCLUDE_SEARCH_DIRS})
ENDIF( NOT $ENV{SMC_INCLUDEDIR} STREQUAL "" )

IF( NOT $ENV{SMC_BINARYDIR} STREQUAL "" )
  SET(_smc_BINARY_SEARCH_DIRS $ENV{SMC_BINARYDIR} ${_smc_BINARY_SEARCH_DIRS})
ENDIF( NOT $ENV{SMC_BINARYDIR} STREQUAL "" )

IF( SMC_HOME )
    SET(_smc_INCLUDE_SEARCH_DIRS ${SMC_HOME}/lib/C++ ${_smc_INCLUDE_SEARCH_DIRS})
    SET(_smc_BINARY_SEARCH_DIRS ${SMC_HOME}/bin ${_smc_BINARY_SEARCH_DIRS})
    SET(_smc_HOME ${SMC_HOME})
ENDIF( SMC_HOME )

# find the include files
FIND_PATH(SMC_INCLUDE_DIR
  NAMES statemap.h
  HINTS ${_smc_INCLUDE_SEARCH_DIRS}
)

FIND_FILE(SMC_JAR
  NAMES Smc.jar
  HINTS ${_smc_BINARY_SEARCH_DIRS}
#    "[HKEY_CURRENT_USER\\smc\\bin]"
  DOC "Location of the Smc.jar file"
)

# if the include and the program are found then we have it
IF(SMC_JAR AND SMC_INCLUDE_DIR)
  SET(SMC_FOUND "YES")
ENDIF(SMC_JAR AND SMC_INCLUDE_DIR)

MARK_AS_ADVANCED(
  SMC_JAR
  SMC_INCLUDE_DIR
  SMC_HOME
)

# Commands beyond this point should not need to know the version.
SET(CMAKE_IMPORT_FILE_VERSION)
CMAKE_POLICY(POP)
