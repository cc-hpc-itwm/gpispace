# -*- mode: cmake; -*-
# Locate the SMC (State Machine Compiler)
# Smc can be found at http://smc.sourceforge.net/
# Written by Alexander Petry, petry _at_ itwm.fhg.de

# This module defines
# SMC_JAR, where is the smc jar file
# SMC_FOUND, If false, don't try to use smc
#
# Use the SMC_HOME environment variable to define the installation directory of SMC

Include(FindJava)

IF( NOT SMC_HOME )
  SET(SMC_HOME "")
ENDIF(NOT SMC_HOME)

FIND_FILE(SMC_JAR
  NAMES
    Smc.jar
  PATHS
    "[HKEY_CURRENT_USER\\smc\\bin]"
    /usr/local/bin
    /usr/bin
    $ENV{SMC_HOME}
    ${SMC_HOME}
  PATH_SUFFIXES
    bin
  DOC "Location of the Smc.jar file"
)

FIND_PATH(SMC_INCLUDE_DIR statemap.h
  "[HKEY_CURRENT_USER\\smc\\lib]"
  $ENV{SMC_HOME}/lib/C++
  ${SMC_HOME}/lib/
  /usr/local/include
  /usr/include
)


# if the include and the program are found then we have it
IF(SMC_JAR)
  SET(SMC_FOUND "YES")
ENDIF(SMC_JAR)

MARK_AS_ADVANCED(
  SMC_JAR
  SMC_INCLUDE_DIR
)
