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

Include(FindJava)

IF( NOT SMC_HOME )
  SET(SMC_HOME "/usr/local/lib/smc")
ENDIF(NOT SMC_HOME)

FIND_FILE(SMC_JAR
  NAMES
    Smc.jar
  PATHS
    $ENV{SMC_HOME}
    ${SMC_HOME}
    "[HKEY_CURRENT_USER\\smc\\bin]"
    /usr/local/bin
    /usr/bin
  PATH_SUFFIXES
    bin
  DOC "Location of the Smc.jar file"
)

FIND_PATH(SMC_INCLUDE_DIR statemap.h
  $ENV{SMC_HOME}/lib/C++
  ${SMC_HOME}/lib/
  "[HKEY_CURRENT_USER\\smc\\lib]"
  /usr/local/include
  /usr/include
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
