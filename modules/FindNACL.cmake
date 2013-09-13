# -*- mode: cmake; -*-
# locates the libnacl library
# This file defines:
# * NACL_FOUND if libreadline was found
# * NACL_LIBRARY The lib to link to (currently only a static unix lib)
# * NACL_STATIC_LIBRARY
# * NACL_SHARED_LIBRARY The lib to link to (currently only a static unix lib)
# * NACL_INCLUDE_DIR
# * NACL_LD_PATH the path to NACL_LIBRARY

if (NOT NACL_FIND_QUIETLY)
  message(STATUS "FindNACL check")
endif ()

find_path (NACL_INCLUDE_DIR
  NAMES "crypto_box.h"
  HINTS ${NACL_HOME} ENV NACL_HOME
  PATH_SUFFIXES include include/nacl
  )

find_library (NACL_STATIC_LIBRARY
  NAMES libnacl.a
  HINTS ${NACL_HOME} ENV NACL_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (NACL_SHARED_LIBRARY
  NAMES libnacl.so
  HINTS ${NACL_HOME} ENV NACL_HOME
  PATH_SUFFIXES lib lib64
  )

if (NACL_STATIC_LIBRARY)
  set(NACL_LIBRARY ${NACL_STATIC_LIBRARY})
else()
  if (NACL_SHARED_LIBRARY)
    set(NACL_LIBRARY ${NACL_SHARED_LIBRARY})
  endif()
endif()

if (NACL_INCLUDE_DIR AND NACL_LIBRARY)
  set (NACL_FOUND TRUE)
  get_filename_component (NACL_LD_PATH ${NACL_LIBRARY} PATH)
  if (NOT NACL_FIND_QUIETLY)
    message (STATUS "Found NACL headers in ${NACL_INCLUDE_DIR} and libraries ${NACL_LIBRARY} ${NACL_SHARED_LIBRARY}")
  endif ()
else ()
  if (NACL_FIND_REQUIRED)
    message (FATAL_ERROR "NACL could not be found!")
  endif ()
endif ()
