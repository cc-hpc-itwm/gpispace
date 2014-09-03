# locates the libgaspi library
# This file defines:
# * GASPI_FOUND if libreadline was found
# * GASPI_LIBRARY The lib to link to (currently only a static unix lib)
# * GASPI_STATIC_LIBRARY
# * GASPI_SHARED_LIBRARY The lib to link to (currently only a static unix lib)
# * GASPI_INCLUDE_DIR
# * GASPI_LD_PATH the path to GASPI_LIBRARY

include (FindPackageHandleStandardArgs)

find_path (GASPI_INCLUDE_DIR
  NAMES "GASPI.h"
  HINTS ${GASPI_HOME} ENV GASPI_HOME
  PATH_SUFFIXES include
  )

find_library (GASPI_STATIC_LIBRARY
  NAMES libGPI2.a
  HINTS ${GASPI_HOME} ENV GASPI_HOME
  PATH_SUFFIXES lib lib64
  )
set(GASPI_LIBRARY ${GASPI_STATIC_LIBRARY})

mark_as_advanced (GASPI_INCLUDE_DIR GASPI_STATIC_LIBRARY GASPI_LIBRARY)

if (GASPI_INCLUDE_DIR AND NOT GASPI_VERSION_STRING)
  execute_process (
    COMMAND grep -o "#define GASPI_MAJOR_VERSION ([0-9])\\|#define GASPI_MINOR_VERSION ([0-9])\\|#define GASPI_REVISION ([0-9])"
    COMMAND awk "{print $3}"
    COMMAND tr -d "()"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    INPUT_FILE "${GASPI_INCLUDE_DIR}/GASPI.h"
    RESULT_VARIABLE _res
    ERROR_VARIABLE _err
    OUTPUT_VARIABLE _out
    TIMEOUT 30
  )
  if ("${_res}" EQUAL "0")
    string (REPLACE "\n" ";" VERSION_LIST ${_out})
    list (GET VERSION_LIST 0 GASPI_VERSION_MAJOR)
    list (GET VERSION_LIST 1 GASPI_VERSION_MINOR)
    list (GET VERSION_LIST 2 GASPI_VERSION_PATCH)
    set (GASPI_VERSION_STRING "${GASPI_VERSION_MAJOR}.${GASPI_VERSION_MINOR}.${GASPI_VERSION_PATCH}")
    mark_as_advanced (GASPI_VERSION_STRING)
  else()
    message (STATUS "could not get version: ${_res}: ${_err}")
  endif()
endif()

find_package_handle_standard_args (GASPI
  REQUIRED_VARS GASPI_INCLUDE_DIR GASPI_STATIC_LIBRARY GASPI_LIBRARY GASPI_VERSION_STRING
  VERSION_VAR GASPI_VERSION_STRING
)

#if (GASPI_INCLUDE_DIR AND GASPI_LIBRARY)
#  set (GASPI_FOUND TRUE)
#  get_filename_component (GASPI_LD_PATH ${GASPI_LIBRARY} PATH)
#  if (NOT GASPI_FIND_QUIETLY)
#    message (STATUS "Found GASPI headers in ${GASPI_INCLUDE_DIR} and libraries ${GASPI_LIBRARY} ${GASPI_SHARED_LIBRARY}")
#  endif ()
#else ()
#  if (GASPI_FIND_REQUIRED)
#    message (FATAL_ERROR "GASPI could not be found!")
#  endif ()
#endif ()
