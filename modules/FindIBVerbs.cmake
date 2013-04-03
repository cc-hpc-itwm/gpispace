# -*- mode: cmake; -*-
# locates the ibverbs library
# This file defines:
# * IBverbs_FOUND if libibverbs was found
# * IBverbs_LIBRARY The lib to link to (currently only a static unix lib)
# * IBverbs_LIBRARY_SHARED The lib to link to (currently only a static unix lib)
# * IBverbs_INCLUDE_DIR

if (NOT IBVerbs_FIND_QUIETLY)
  message(STATUS "FindIBverbs check")
endif (NOT IBVerbs_FIND_QUIETLY)

find_path (IBverbs_INCLUDE_DIR
  NAMES "infiniband/verbs.h"
  HINTS ${LIBIBVERBS_HOME} ENV LIBIBVERBS_HOME
  PATH_SUFFIXES include
  )

find_library (IBverbs_LIBRARY
  NAMES ibverbs
  HINTS ${LIBIBVERBS_HOME} ENV LIBIBVERBS_HOME
  PATH_SUFFIXES lib lib64
  )

if (IBverbs_INCLUDE_DIR AND IBverbs_LIBRARY)
  set (IBverbs_FOUND TRUE)
  if (NOT IBVerbs_FIND_QUIETLY)
    message (STATUS "Found IBverbs headers in ${IBverbs_INCLUDE_DIR} and libraries ${IBverbs_LIBRARY} ${IBverbs_LIBRARY_SHARED}")
  endif (NOT IBVerbs_FIND_QUIETLY)
else (IBverbs_INCLUDE_DIR AND IBverbs_LIBRARY)
  if (IBVerbs_FIND_REQUIRED)
    message (FATAL_ERROR "IBverbs could not be found!")
  endif (IBVerbs_FIND_REQUIRED)
endif (IBverbs_INCLUDE_DIR AND IBverbs_LIBRARY)
