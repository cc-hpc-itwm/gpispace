# -*- mode: cmake; -*-
#
#  Helper to find packages / library and include dir
#
#
# <package>_FOUND if <package> was found
# <package>_LIBRARY
# <package>_LIBRARIES
# <package>_LIBRARY_DIR
# <package>_INCLUDE_DIR
#

function(check_package_search_path NAME INCLUDE_FILE LIBNAME LIBVERSION
   GEN_INCLUDE_SEARCH_DIRS
   GEN_LIBRARIES_SEARCH_DIRS
   GEN_BINARY_SEARCH_DIRS
   GEN_HOME)

  string(TOLOWER ${NAME} _name)

  IF (NOT WIN32)
    include(FindPkgConfig)
    if ( PKG_CONFIG_FOUND )

#       pkg_check_modules (PC_${NAME} ${_name}>=${LIBVERSION})

       set(${NAME}_DEFINITIONS ${PC_${NAME}_CFLAGS_OTHER})
    endif(PKG_CONFIG_FOUND)
  endif (NOT WIN32)

# set defaults
  SET(_${_name}_HOME "/usr/local")
  SET(${GEN_INCLUDE_SEARCH_DIRS}
    ${CMAKE_INCLUDE_PATH}
    /usr/local/include
    /usr/include
    )

  SET(${GEN_LIBRARIES_SEARCH_DIRS}
    ${CMAKE_LIBRARY_PATH}
    /usr/local/lib
    /usr/lib
    )

##
  if( "${${NAME}_HOME}" STREQUAL "")
    if("" MATCHES "$ENV{${NAME}_HOME}")
      message(STATUS "${NAME}_HOME env is not set, setting it to /usr/local")
      set (${NAME}_HOME ${_${_name}_HOME})
    else("" MATCHES "$ENV{${NAME}_HOME}")
      set (${NAME}_HOME "$ENV{${NAME}_HOME}")
    endif("" MATCHES "$ENV{${NAME}_HOME}")
  else( "${${NAME}_HOME}" STREQUAL "")
    message(STATUS "${NAME}_HOME is not empty: \"${${NAME}_HOME}\"")
  endif( "${${NAME}_HOME}" STREQUAL "")
##

  message(STATUS "Looking for ${_name} in ${${NAME}_HOME}")

  IF( NOT ${${NAME}_HOME} STREQUAL "" )
    SET(${GEN_INCLUDE_SEARCH_DIRS} ${${NAME}_HOME}/include ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_BINARY_SEARCH_DIRS} ${${NAME}_HOME}/bin ${${GEN_BINRAY_SEARCH_DIRS}})
    SET(${GEN_LIBRARIES_SEARCH_DIRS} ${${NAME}_HOME}/lib ${${GEN_LIBRARIES_SEARCH_DIRS}})
    SET(${GEN_HOME} ${${NAME}_HOME})
  ENDIF( NOT ${${NAME}_HOME} STREQUAL "" )

  IF( NOT $ENV{${NAME}_INCLUDEDIR} STREQUAL "" )
    SET(${GEN_INCLUDE_SEARCH_DIRS} $ENV{${NAME}_INCLUDEDIR} ${${GEN_INCLUDE_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_INCLUDEDIR} STREQUAL "" )
 
  IF( NOT $ENV{${NAME}_BINARYDIR} STREQUAL "" )
    SET(${GEN_BINARY_SEARCH_DIRS} $ENV{${NAME}_BINARYDIR} ${${GEN_BINARY_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_BINARYDIR} STREQUAL "" )

  IF( NOT $ENV{${NAME}_LIBRARYDIR} STREQUAL "" )
    SET(${GEN}_LIBRARIES_SEARCH_DIRS $ENV{${NAME}_LIBRARYDIR} ${${GEN_LIBRARIES_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_LIBRARYDIR} STREQUAL "" )

  IF( ${NAME}_HOME )
    SET(${GEN_INCLUDE_SEARCH_DIRS} ${${NAME}_HOME}/include ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_BINARY_SEARCH_DIRS} ${${NAME}_HOME}/bin ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_LIBRARIES_SEARCH_DIRS} ${${NAME}_HOME}/lib ${${GEN_LIBRARIES_SEARCH_DIRS}})
    SET(${GEN_HOME} ${${NAME}_HOME})
  ENDIF( ${NAME}_HOME )

  set(${GEN_HOME} ${${GEN_HOME}} PARENT_SCOPE)
  set(${GEN_INCLUDE_SEARCH_DIRS} ${${GEN_INCLUDE_SEARCH_DIRS}} PARENT_SCOPE)
  set(${GEN_LIBRARIES_SEARCH_DIRS} ${${GEN_LIBRARIES_SEARCH_DIRS}} PARENT_SCOPE)
  set(${GEN_BINARY_SEARCH_DIRS} ${${GEN_BINARY_SEARCH_DIRS}} PARENT_SCOPE)
  message(STATUS "Looking for HOME in ${${GEN_HOME}}")
  message(STATUS "Looking for include in in '${${GEN_INCLUDE_SEARCH_DIRS}}'")
  message(STATUS "Looking for library in in '${${GEN_LIBRARIES_SEARCH_DIRS}}'")
  message(STATUS "Looking for binary in in '${${GEN_BINARY_SEARCH_DIRS}}'")

endfunction()

#
#
#
function(check_package NAME INCLUDE_FILE LIBNAME LIBVERSION)
  message(STATUS "check for package ${NAME}")
  string(TOLOWER ${NAME} _name)

  check_package_search_path(${NAME} ${INCLUDE_FILE} ${LIBNAME} ${LIBVERSION}
    _xxx_INCLUDE_SEARCH_DIRS
    _xxx_LIBRARIES_SEARCH_DIRS
    _xxx_BINARY_SEARCH_DIRS
    _xxx_HOME)


#  message(STATUS "Looking for HOME in '${_xxx_HOME}'")
#  message(STATUS "Looking for include in in '${_xxx_INCLUDE_SEARCH_DIRS}'")
#  message(STATUS "Looking for library in in '${_xxx_LIBRARIES_SEARCH_DIRS}'")
#  message(STATUS "Looking for binary in in '${_xxx_BINARY_SEARCH_DIRS}'")
   
  # find the include files
  FIND_PATH(${NAME}_INCLUDE_DIR ${INCLUDE_FILE}
    HINTS
    ${_xxx_INCLUDE_SEARCH_DIRS}
    ${PC_${NAME}_INCLUDEDIR}
    ${PC_${NAME}_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
    )

  # locate the library
  SET(${NAME}_LIBRARY_NAMES ${${NAME}_LIBRARY_NAMES} ${LIBNAME})
  IF(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  ELSE(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  ENDIF(WIN32)
  message("  search for library '${${NAME}_LIBRARY_NAMES}'")
  FIND_LIBRARY(${NAME}_LIBRARY
    NAMES ${${NAME}_LIBRARY_NAMES}
    HINTS
    ${_xxx_LIBRARIES_SEARCH_DIRS}
    ${PC_${NAME}_LIBDIR}
    ${PC_${NAME}_LIBRARY_DIRS}
    )

  # if the include and the program are found then we have it
  IF(${NAME}_LIBRARY AND ${NAME}_INCLUDE_DIR) 
    message(STATUS "Found ${_name}: -I${${NAME}_INCLUDE_DIR} ${${NAME}_LIBRARY}")
    GET_FILENAME_COMPONENT (${NAME}_LIBRARY_DIR ${${NAME}_LIBRARY} PATH)
    GET_FILENAME_COMPONENT (${NAME}_LIBRARIES ${${NAME}_LIBRARY} NAME)
    set(${NAME}_LIBRARY_DIR ${${NAME}_LIBRARY_DIR} PARENT_SCOPE)
    SET(${NAME}_FOUND true PARENT_SCOPE)
  ENDIF(${NAME}_LIBRARY AND ${NAME}_INCLUDE_DIR)

  message("  ${NAME} 2: -I${${NAME}_INCLUDE_DIR} -L${${NAME}_LIBRARY_DIR} ")
  message("             ${${NAME}_LIBRARIES} ${${NAME}_LIBRARY}")

  MARK_AS_ADVANCED(
    ${NAME}_FOUND
    ${NAME}_LIBRARY
    ${NAME}_LIBRARIES
    ${NAME}_LIBRARY_DIR
    ${NAME}_INCLUDE_DIR
    )

#endfunction(check_package NAME INCLUDE_FILE LIBNAME LIBVERSION)
endfunction()
