# -*- mode: cmake; -*-
# locates the liblua library
# This file defines:
# * LUA_FOUND if libreadline was found
# * LUA_LIBRARY The lib to link to (currently only a static unix lib)
# * LUA_STATIC_LIBRARY
# * LUA_SHARED_LIBRARY The lib to link to (currently only a static unix lib)
# * LUA_INCLUDE_DIR

if (NOT LUA_FIND_QUIETLY)
  message(STATUS "FindLUA check")
endif ()

find_path (LUA_INCLUDE_DIR
  NAMES "lua.hpp"
  HINTS ${LUA_HOME} ENV LUA_HOME
  PATH_SUFFIXES include
  )

find_library (LUA_STATIC_LIBRARY
  NAMES liblua.a
  HINTS ${LUA_HOME} ENV LUA_HOME
  PATH_SUFFIXES lib lib64
  )
find_library (LUA_SHARED_LIBRARY
  NAMES liblua.so
  HINTS ${LUA_HOME} ENV LUA_HOME
  PATH_SUFFIXES lib lib64
  )

if (LUA_STATIC_LIBRARY)
  set(LUA_LIBRARY ${LUA_STATIC_LIBRARY})
else()
  if (LUA_SHARED_LIBRARY)
    set(LUA_LIBRARY ${LUA_SHARED_LIBRARY})
  endif()
endif()

if (LUA_INCLUDE_DIR AND LUA_LIBRARY)
  set (LUA_FOUND TRUE)
  if (NOT LUA_FIND_QUIETLY)
    message (STATUS "Found LUA headers in ${LUA_INCLUDE_DIR} and libraries ${LUA_STATIC_LIBRARY} ${LUA_SHARED_LIBRARY}")
  endif ()
else ()
  if (LUA_FIND_REQUIRED)
    message (FATAL_ERROR "LUA could not be found!")
  endif ()
endif ()
