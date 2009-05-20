# Tries to locate a logging framework.
# This file defines:
# * LOG4CPP_FOUND if log4cpp was found
# * LOG4CPP_LIBRARIES The lib to link to (currently only a static unix lib, not portable) 
# * LOG4CPP_LIBRARY_DIRS The path to the library directory
# * LOG4CPP_INCLUDE_DIRS The path to the include directory

if ( PKG_CONFIG_FOUND )

   pkg_check_modules (LOG4CPP REQUIRED log4cpp>=1.0)

else  ( PKG_CONFIG_FOUND )

  FIND_PATH(LOG4CPP_INCLUDE_DIRS log4cpp/Category.hh
    ${CMAKE_INCLUDE_PATH}
    $ENV{LOG4CPP_HOME}/include
    /usr/local/include
    /usr/include
  )

  SET(LOG4CPP_LIBRARY_NAMES ${LOG4CPP_LIBRARY_NAMES} liblog4cpp.a)
  FIND_LIBRARY(LOG4CPP_LIBRARY
    NAMES ${LOG4CPP_LIBRARY_NAMES}
    PATHS ${CMAKE_LIBRARY_PATH} $ENV{LOG4CPP_HOME}/lib /usr/lib /usr/local/lib
  )


# if the include and the program are found then we have it
  IF(LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIRS) 
    message(STATUS "Found log4cpp: -I${LOG4CPP_INCLUDE_DIRS} ${LOG4CPP_LIBRARY}")
    GET_FILENAME_COMPONENT (LOG4CPP_LIBRARY_DIRS ${LOG4CPP_LIBRARY} PATH)
    GET_FILENAME_COMPONENT (LOG4CPP_LIBRARIES ${LOG4CPP_LIBRARY} NAME)
    SET(LOG4CPP_FOUND "YES")
  ENDIF(LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIRS)
ENDIF(PKG_CONFIG_FOUND)

MARK_AS_ADVANCED(
  LOG4CPP_LIBRARY
  LOG4CPP_LIBRARIES
  LOG4CPP_LIBRARY_DIRS
  LOG4CPP_INCLUDE_DIRS
)
