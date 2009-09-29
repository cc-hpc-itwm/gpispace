# locates the Gwes Workflow Engine
# This file defines:
# * GWES_FOUND if gwes was was found
# * GWES_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWES_INCLUDE_DIR
#
# * GWDL_FOUND if gwdl was was found
# * GWDL_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWDL_INCLUDE_DIR

if(GWES_HOME MATCHES "")
  if("" MATCHES "$ENV{GWES_HOME}")
    message(STATUS "GWES_HOME env is not set, setting it to /usr/local")
    set (GWES_HOME "/usr/local")
  else("" MATCHES "$ENV{GWES_HOME}")
    set (GWES_HOME "$ENV{GWES_HOME}")
  endif("" MATCHES "$ENV{GWES_HOME}")
else(GWES_HOME MATCHES "")
  message(STATUS "GWES_HOME is not empty: \"${GWES_HOME}\"")
  set (GWES_HOME "${GWES_HOME}")
endif(GWES_HOME MATCHES "")

FIND_PATH(GWES_INCLUDE_DIR gwes/GWES.h
  ${GWES_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
)

IF(WIN32)
  SET(GWES_LIBRARY_NAMES ${GWES_LIBRARY_NAMES} libgwes_cpp.lib)
ELSE(WIN32)
  SET(GWES_LIBRARY_NAMES ${GWES_LIBRARY_NAMES} libgwes_cpp.a libgwes_cpp.so)
ENDIF(WIN32)

FIND_LIBRARY(GWES_LIBRARY
  NAMES ${GWES_LIBRARY_NAMES}
  PATHS ${GWES_HOME}/lib ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
  )

# if the include and the program are found then we have it
IF(GWES_INCLUDE_DIR AND GWES_LIBRARY) 
  SET(GWES_FOUND "YES")
  message(STATUS "Looking for GWES ... found headers in ${GWES_INCLUDE_DIR} and library ${GWES_LIBRARY}")
else(GWES_INCLUDE_DIR AND GWES_LIBRARY) 
  message(STATUS "GWES could not be found, try setting GWES_HOME (value=\"${GWES_HOME}\").")
ENDIF(GWES_INCLUDE_DIR AND GWES_LIBRARY)

MARK_AS_ADVANCED(
  GWES_HOME
  GWES_LIBRARY
  GWES_INCLUDE_DIR
)

##
##
## GWDL stuff
##
##

if(GWDL_HOME MATCHES "")
  if("" MATCHES "$ENV{GWDL_HOME}")
    message(STATUS "GWDL_HOME env is not set, setting it to /usr/local")
    set (GWDL_HOME "/usr/local")
  else("" MATCHES "$ENV{GWDL_HOME}")
    set (GWDL_HOME "$ENV{GWDL_HOME}")
  endif("" MATCHES "$ENV{GWDL_HOME}")
else(GWDL_HOME MATCHES "")
  message(STATUS "GWDL_HOME is not empty: \"${GWDL_HOME}\"")
  set (GWDL_HOME "${GWDL_HOME}")
endif(GWDL_HOME MATCHES "")

FIND_PATH(GWDL_INCLUDE_DIR gwdl/Workflow.h
  ${GWDL_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
)

IF(WIN32)
  SET(GWDL_LIBRARY_NAMES ${GWDL_LIBRARY_NAMES} libgworkflowdl_cpp.lib)
ELSE(WIN32)
  SET(GWDL_LIBRARY_NAMES ${GWDL_LIBRARY_NAMES} libgworkflowdl_cpp.a libgworkflowdl_cpp.so)
ENDIF(WIN32)

FIND_LIBRARY(GWDL_LIBRARY
  NAMES ${GWDL_LIBRARY_NAMES}
  PATHS ${GWDL_HOME}/lib ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
  )

# if the include and the program are found then we have it
IF(GWDL_INCLUDE_DIR AND GWDL_LIBRARY) 
  SET(GWDL_FOUND "YES")
  message(STATUS "Looking for GWDL ... found headers in ${GWDL_INCLUDE_DIR} and library ${GWDL_LIBRARY}")
else(GWDL_INCLUDE_DIR AND GWDL_LIBRARY) 
  message(STATUS "GWDL could not be found, try setting GWDL_HOME (value=\"${GWDL_HOME}\").")
ENDIF(GWDL_INCLUDE_DIR AND GWDL_LIBRARY)

MARK_AS_ADVANCED(
  GWDL_HOME
  GWDL_LIBRARY
  GWDL_INCLUDE_DIR
)

