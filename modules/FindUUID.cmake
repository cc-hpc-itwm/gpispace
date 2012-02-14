# Tries to find the uuid library
# This file defines:
# * UUID_FOUND if protoc was found
# * UUID_LIBRARY The lib to link to (currently only a static unix lib, not portable) 

if(UUID_HOME MATCHES "")
  if("" MATCHES "$ENV{UUID_HOME}")
    set (UUID_HOME "/usr/local")
  else("" MATCHES "$ENV{UUID_HOME}")
    set (UUID_HOME "$ENV{UUID_HOME}")
  endif("" MATCHES "$ENV{UUID_HOME}")
else(UUID_HOME MATCHES "")
  set (UUID_HOME "${UUID_HOME}")
endif(UUID_HOME MATCHES "")

FIND_PATH(UUID_INCLUDE_DIR uuid/uuid.h
  ${UUID_HOME}/include
  ${CMAKE_INCLUDE_PATH}
  /usr/local/include
  /usr/include
  )

IF(WIN32)
  SET(UUID_LIBRARY_NAMES ${UUID_LIBRARY_NAMES} libuuid.lib)
ELSE(WIN32)
  SET(UUID_LIBRARY_NAMES ${UUID_LIBRARY_NAMES} libuuid.a libuuid.so)
ENDIF(WIN32)

FIND_LIBRARY(UUID_LIBRARY
  NAMES ${UUID_LIBRARY_NAMES}
  PATHS ${UUID_HOME}/lib ${CMAKE_LIBRARY_PATH} /usr/lib /usr/local/lib
  )

# if the include and the program are found then we have it
IF(UUID_INCLUDE_DIR AND UUID_LIBRARY) 
  SET(UUID_FOUND "YES")
  message(STATUS "Found UUID Inc:${UUID_INCLUDE_DIR} Lib:${UUID_LIBRARY}")
else(UUID_INCLUDE_DIR AND UUID_LIBRARY) 
  message(STATUS "UUID library could not be found, try setting UUID_HOME (value=\"${UUID_HOME}\").")
ENDIF(UUID_INCLUDE_DIR AND UUID_LIBRARY)

MARK_AS_ADVANCED(
  UUID_HOME
  UUID_LIBRARY
  UUID_INCLUDE_DIR
)
