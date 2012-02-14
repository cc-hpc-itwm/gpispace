# -*- mode: cmake; -*-
# locates the libreadline
# This file defines:
# * LibReadline_FOUND if libreadline was found
# * LibReadline_LIBRARY The lib to link to (currently only a static unix lib)
# * LibReadline_LIBRARY_SHARED The lib to link to (currently only a static unix lib)
# * LibReadline_INCLUDE_DIR

if (NOT LibReadline_FIND_QUIETLY)
  message(STATUS "FindLibReadline check")
endif (NOT LibReadline_FIND_QUIETLY)

find_path (LibReadline_INCLUDE_DIR
  NAMES "readline/readline.h"
  HINTS ${LIBREADLINE_HOME} ENV LIBREADLINE_HOME
  PATH_SUFFIXES include
  )

find_library (LibReadline_LIBRARY
  NAMES readline
  HINTS ${LIBREADLINE_HOME} ENV LIBREADLINE_HOME
  PATH_SUFFIXES lib lib64
  )
find_library(LibTermcap_LIBRARY
  NAMES termcap
  HINTS ${LIBTERMCAP_HOME} ENV LIBTERMCAP_HOME
  PATH_SUFFIXES lib lib64
)
if (LibTermcap_LIBRARY)
  set(LibReadline_LIBRARY ${LibReadline_LIBRARY} ${LibTermcap_LIBRARY})
endif (LibTermcap_LIBRARY)

if (LibReadline_INCLUDE_DIR AND LibReadline_LIBRARY)
  set (LibReadline_FOUND TRUE)
  if (NOT LibReadline_FIND_QUIETLY)
    message (STATUS "Found LibReadline headers in ${LibReadline_INCLUDE_DIR} and libraries ${LibReadline_LIBRARY} ${LibReadline_LIBRARY_SHARED}")
  endif (NOT LibReadline_FIND_QUIETLY)
else (LibReadline_INCLUDE_DIR AND LibReadline_LIBRARY)
  if (LibReadline_FIND_REQUIRED)
    message (FATAL_ERROR "LibReadline could not be found!")
  endif (LibReadline_FIND_REQUIRED)
endif (LibReadline_INCLUDE_DIR AND LibReadline_LIBRARY)
