# -*- mode: cmake; -*-
# locates the libreadline
# This file defines:
# * Readline_FOUND if libreadline was found
# * Readline_LIBRARY The lib to link to (currently only a static unix lib)
# * Readline_LIBRARY_SHARED The lib to link to (currently only a static unix lib)
# * Readline_INCLUDE_DIR

if (NOT Readline_FIND_QUIETLY)
  message(STATUS "FindReadline check")
endif (NOT Readline_FIND_QUIETLY)

find_path (Readline_INCLUDE_DIR
  NAMES "readline/readline.h"
  HINTS ${READLINE_HOME} ENV READLINE_HOME
  PATH_SUFFIXES include
  )

find_library (Readline_LIBRARY
  NAMES libreadline.a
  HINTS ${READLINE_HOME} ENV READLINE_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (Readline_LIBRARY_SHARED
  NAMES libreadline.so
  HINTS ${READLINE_HOME} ENV READLINE_HOME
  PATH_SUFFIXES lib lib64
  )

if (Readline_INCLUDE_DIR AND Readline_LIBRARY)
  set (Readline_FOUND TRUE)
  if (NOT Readline_FIND_QUIETLY)
    message (STATUS "Found Readline headers in ${Readline_INCLUDE_DIR} and libraries ${Readline_LIBRARY} ${Readline_LIBRARY_SHARED}")
  endif (NOT Readline_FIND_QUIETLY)
else (Readline_INCLUDE_DIR AND Readline_LIBRARY)
  if (Readline_FIND_REQUIRED)
    message (FATAL_ERROR "Readline could not be found!")
  endif (Readline_FIND_REQUIRED)
endif (Readline_INCLUDE_DIR AND Readline_LIBRARY)
