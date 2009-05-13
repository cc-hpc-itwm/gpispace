# Tries to locate a logging framework.
# This file defines:
# * LOG4CPP_FOUND if log4cpp was found
# * LOG4CPP_LIBRARY The lib to link to (currently only a static unix lib, not portable) 
# * LOG4CPP_INCLUDE_DIR The path to the include directory

FIND_PATH(LOG4CPP_INCLUDE_DIR log4cpp/Category.hh
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
IF(LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIR) 
  message(STATUS "Found log4cpp: -I${LOG4CPP_INCLUDE_DIR} -l${LOG4CPP_LIBRARY}")
  SET(LOG4CPP_FOUND "YES")
ENDIF(LOG4CPP_LIBRARY AND LOG4CPP_INCLUDE_DIR)

MARK_AS_ADVANCED(
  LOG4CPP_LIBRARY
  LOG4CPP_INCLUDE_DIR
)
