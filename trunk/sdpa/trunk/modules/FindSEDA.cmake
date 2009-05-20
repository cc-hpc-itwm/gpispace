# Tries to locate a logging framework.
# This file defines:
# * SEDA_FOUND if log4cpp was found
# * SEDA_LIBRARY The lib to link to (currently only a static unix lib, not portable) 
# * SEDA_INCLUDE_DIR The path to the include directory
# *  PKG_CHECK_MODULE used to set the following variables
# *
# *   <PREFIX>_FOUND           ... set to 1 if module(s) exist
# *   <XPREFIX>_LIBRARIES      ... only the libraries (w/o the '-l')
# *   <XPREFIX>_LIBRARY_DIRS   ... the paths of the libraries (w/o the '-L')
# *   <XPREFIX>_LDFLAGS        ... all required linker flags
# *   <XPREFIX>_LDFLAGS_OTHER  ... all other linker flags
# *   <XPREFIX>_INCLUDE_DIRS   ... the '-I' preprocessor flags (w/o the '-I')
# *   <XPREFIX>_CFLAGS         ... all required cflags
# *   <XPREFIX>_CFLAGS_OTHER   ... the other compiler flags

# *   <XPREFIX> = <PREFIX>_STATIC for static linking
# *   <XPREFIX>_VERSION    ... version of the module
# *   <XPREFIX>_PREFIX     ... prefix-directory of the module
# *   <XPREFIX>_INCLUDEDIR ... include-dir of the module
# *   <XPREFIX>_LIBDIR     ... lib-dir of the module
# *   <XPREFIX> = <PREFIX>  when |MODULES| == 1, else
# *   <XPREFIX> = <PREFIX>_<MODNAME>

if ( PKG_CONFIG_FOUND )

   pkg_check_modules (SEDA REQUIRED libseda>=0.1)
   message(STATUS "SEDA: -I${SEDA_INCLUDE_DIRS} -L${SEDA_LIBRARY_DIRS} -l${SEDA_LIBRARIES} f=${SEDA_FOUND}")

else  ( PKG_CONFIG_FOUND )
  FIND_PATH(SEDA_INCLUDE_DIRS seda/IEvent.hpp
    ${CMAKE_INCLUDE_PATH}
    $ENV{SEDA_HOME}/include
    /usr/local/include
    /usr/include
  )

SET(SEDA_LIBRARY_NAMES ${SEDA_LIBRARY_NAMES} libseda.a)
FIND_LIBRARY(SEDA_LIBRARY
  NAMES ${SEDA_LIBRARY_NAMES}
  PATHS ${CMAKE_LIBRARY_PATH} $ENV{SEDA_HOME}/lib /usr/lib /usr/local/lib
)


# if the include and the program are found then we have it
  IF(SEDA_LIBRARY AND SEDA_INCLUDE_DIRS) 
    message(STATUS "Found seda: -I${SEDA_INCLUDE_DIRS} ${SEDA_LIBRARY}")
    GET_FILENAME_COMPONENT (SEDA_LIBRARY_DIRS ${SEDA_LIBRARY} PATH)
    GET_FILENAME_COMPONENT (SEDA_LIBRARIES ${SEDA_LIBRARY} NAME)
    SET(SEDA_FOUND "YES")
  ENDIF(SEDA_LIBRARY AND SEDA_INCLUDE_DIRS)
ENDIF(PKG_CONFIG_FOUND)

MARK_AS_ADVANCED(
  SEDA_LIBRARY
  SEDA_LIBRARIES
  SEDA_LIBRARY_DIRS
  SEDA_INCLUDE_DIRS
)
