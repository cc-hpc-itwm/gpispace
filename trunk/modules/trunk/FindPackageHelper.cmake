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
  message(STATUS "  build searchpath for package ${NAME}")

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

  SET(${GEN_BINARY_SEARCH_DIRS}
    ${CMAKE_BINARY_PATH}
    /usr/local/lib
    /usr/lib
    )
  #

  if( "${${NAME}_HOME}" STREQUAL "")
    if("" MATCHES "$ENV{${NAME}_HOME}")
      message(STATUS "    ${NAME}_HOME env is not set, setting it to /usr/local")
      set (${NAME}_HOME ${_${_name}_HOME})
    else("" MATCHES "$ENV{${NAME}_HOME}")
      set (${NAME}_HOME "$ENV{${NAME}_HOME}")
    endif("" MATCHES "$ENV{${NAME}_HOME}")
  else( "${${NAME}_HOME}" STREQUAL "")
    message(STATUS "    ${NAME}_HOME is not empty: \"${${NAME}_HOME}\"")
  endif( "${${NAME}_HOME}" STREQUAL "")
##

  message(STATUS "    Looking for ${_name} in ${${NAME}_HOME}")

  IF( NOT ${${NAME}_HOME} STREQUAL "" )
    SET(${GEN_INCLUDE_SEARCH_DIRS} ${${NAME}_HOME}/include ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_LIBRARIES_SEARCH_DIRS} ${${NAME}_HOME}/lib ${${GEN_LIBRARIES_SEARCH_DIRS}})
    SET(${GEN_BINARY_SEARCH_DIRS} ${${NAME}_HOME}/bin ${${GEN_BINRAY_SEARCH_DIRS}})
    SET(${GEN_HOME} ${${NAME}_HOME})
  ENDIF( NOT ${${NAME}_HOME} STREQUAL "" )

  IF( NOT $ENV{${NAME}_INCLUDEDIR} STREQUAL "" )
    SET(${GEN_INCLUDE_SEARCH_DIRS} $ENV{${NAME}_INCLUDEDIR} ${${GEN_INCLUDE_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_INCLUDEDIR} STREQUAL "" )
 
  IF( NOT $ENV{${NAME}_LIBRARYDIR} STREQUAL "" )
    SET(${GEN}_LIBRARIES_SEARCH_DIRS $ENV{${NAME}_LIBRARYDIR} ${${GEN_LIBRARIES_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_LIBRARYDIR} STREQUAL "" )

  IF( NOT $ENV{${NAME}_BINARYDIR} STREQUAL "" )
    SET(${GEN_BINARY_SEARCH_DIRS} $ENV{${NAME}_BINARYDIR} ${${GEN_BINARY_SEARCH_DIRS}})
  ENDIF( NOT $ENV{${NAME}_BINARYDIR} STREQUAL "" )

  IF( ${NAME}_HOME )
    SET(${GEN_INCLUDE_SEARCH_DIRS} ${${NAME}_HOME}/include ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_LIBRARIES_SEARCH_DIRS} ${${NAME}_HOME}/lib ${${GEN_LIBRARIES_SEARCH_DIRS}})
    SET(${GEN_BINARY_SEARCH_DIRS} ${${NAME}_HOME}/bin ${${GEN_INCLUDE_SEARCH_DIRS}})
    SET(${GEN_HOME} ${${NAME}_HOME})
  ENDIF( ${NAME}_HOME )

  set(${GEN_HOME} ${${GEN_HOME}} PARENT_SCOPE)
  set(${GEN_INCLUDE_SEARCH_DIRS} ${${GEN_INCLUDE_SEARCH_DIRS}} PARENT_SCOPE)
  set(${GEN_LIBRARIES_SEARCH_DIRS} ${${GEN_LIBRARIES_SEARCH_DIRS}} PARENT_SCOPE)
  set(${GEN_BINARY_SEARCH_DIRS} ${${GEN_BINARY_SEARCH_DIRS}} PARENT_SCOPE)
  message(STATUS "    Looking for HOME in ${${GEN_HOME}}")
  message(STATUS "    Looking for include in '${${GEN_INCLUDE_SEARCH_DIRS}}'")
  message(STATUS "    Looking for library in '${${GEN_LIBRARIES_SEARCH_DIRS}}'")
  message(STATUS "    Looking for binary in '${${GEN_BINARY_SEARCH_DIRS}}'")

endfunction()

#
#
#
function(check_package NAME INCLUDE_FILE LIBNAME LIBVERSION)
  message(STATUS "check for package ${NAME}")
  string(TOLOWER ${NAME} _name)
  string(TOUPPER ${NAME} _NAME_UPPER)

  #if( ${_NAME_UPPER}_FOUND )
  #  message(STATUS "----- Package ${_NAME_UPPER} allready loaded")
  #endif( ${_NAME_UPPER}_FOUND )

  if(${_NAME_UPPER}_LIBRARY) 
    set(${_name}_IN_CACHE true)
    message(STATUS "  Package ${_NAME_UPPER} allready '${${_NAME_UPPER}_LIBRARY}' loaded")
  endif(${_NAME_UPPER}_LIBRARY) 

  check_package_search_path(${_NAME_UPPER} ${INCLUDE_FILE} ${LIBNAME} ${LIBVERSION}
    _xxx_INCLUDE_SEARCH_DIRS
    _xxx_LIBRARIES_SEARCH_DIRS
    _xxx_BINARY_SEARCH_DIRS
    _xxx_HOME)


#  message(STATUS "    Looking for HOME    in '${_xxx_HOME}'")
#  message(STATUS "    Looking for include in '${_xxx_INCLUDE_SEARCH_DIRS}'")
#  message(STATUS "    Looking for library in '${_xxx_LIBRARIES_SEARCH_DIRS}'")
#  message(STATUS "    Looking for binary  in '${_xxx_BINARY_SEARCH_DIRS}'")
   
  # find the include files
  FIND_PATH(${_NAME_UPPER}_INCLUDE_DIR ${INCLUDE_FILE}
    HINTS
    ${_xxx_INCLUDE_SEARCH_DIRS}
    ${PC_${_NAME_UPPER}_INCLUDEDIR}
    ${PC_${_NAME_UPPER}_INCLUDE_DIRS}
    ${CMAKE_INCLUDE_PATH}
    )

  # locate the library
  SET(${_NAME_UPPER}_LIBRARY_NAMES ${${_NAME_UPPER}_LIBRARY_NAMES} ${LIBNAME})
  IF(WIN32)
    SET(${_NAME_UPPER}_LIBRARY_NAMES ${${_NAME_UPPER}_LIBRARY_NAMES} "lib${LIBNAME}")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  ELSE(WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  ENDIF(WIN32)
  message(STATUS "  search for library '${${_NAME_UPPER}_LIBRARY_NAMES}'")
  FIND_LIBRARY(${_NAME_UPPER}_LIBRARY
    NAMES ${${_NAME_UPPER}_LIBRARY_NAMES}
    HINTS
    ${_xxx_LIBRARIES_SEARCH_DIRS}
    ${PC_${_NAME_UPPER}_LIBDIR}
    ${PC_${_NAME_UPPER}_LIBRARY_DIRS}
    )

  # handle the QUIETLY and REQUIRED arguments and set JPEG_FOUND to TRUE if
  # all listed variables are TRUE
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(${_NAME_UPPER} DEFAULT_MSG
    ${_NAME_UPPER}_LIBRARY ${_NAME_UPPER}_INCLUDE_DIR)

  # if the include and the program are found then we have it
  if(${_NAME_UPPER}_FOUND)
    if( NOT ${_NAME_UPPER}_HOME)
      set(${_NAME_UPPER}_HOME ${_xxx_HOME} PARENT_SCOPE)
    endif( NOT ${_NAME_UPPER}_HOME)

    message(STATUS "    Found ${_name}: -I${${_NAME_UPPER}_INCLUDE_DIR} ${${_NAME_UPPER}_LIBRARY}")
    GET_FILENAME_COMPONENT (${_NAME_UPPER}_LIBRARY_DIR ${${_NAME_UPPER}_LIBRARY} PATH)
    GET_FILENAME_COMPONENT (${_NAME_UPPER}_LIBRARIES ${${_NAME_UPPER}_LIBRARY} NAME)
    set(${_NAME_UPPER}_LIBRARY_DIR ${${_NAME_UPPER}_LIBRARY_DIR} PARENT_SCOPE)
    set(${_NAME_UPPER}_FOUND ${${_NAME_UPPER}_FOUND} PARENT_SCOPE)
  endif(${_NAME_UPPER}_FOUND)

  message(STATUS "    ${_NAME_UPPER} 2: '-I${${_NAME_UPPER}_INCLUDE_DIR}' '-L${${_NAME_UPPER}_LIBRARY_DIR}' ")
  message(STATUS "             '${${_NAME_UPPER}_LIBRARIES}' '${${_NAME_UPPER}_LIBRARY}'")
  message(STATUS "    search '${${_NAME_UPPER}_LIBRARY_DIR}/shared/lib${_name}Config.cmake'")

  #
  if( NOT ${_name}_IN_CACHE )
    if(EXISTS ${${_NAME_UPPER}_LIBRARY_DIR}/shared/lib${_name}Config.cmake)
      message(STATUS "    Include ${_NAME_UPPER} dependencies.")
      include(${${_NAME_UPPER}_LIBRARY_DIR}/shared/lib${_name}Config.cmake)
      set(${_NAME_UPPER}_LIBRARY ${_name} PARENT_SCOPE)
      set(${_NAME_UPPER}_INCLUDE_DIRS ${${_NAME_UPPER}_INCLUDE_DIRS} PARENT_SCOPE)
    endif(EXISTS ${${_NAME_UPPER}_LIBRARY_DIR}/shared/lib${_name}Config.cmake)
  else( NOT ${_name}_IN_CACHE )
    message(STATUS "    package ${NAME} was allready in loaded. Do not perform dependencies.")
  endif( NOT ${_name}_IN_CACHE )


  MARK_AS_ADVANCED(
    ${_NAME_UPPER}_HOME
    ${_NAME_UPPER}_FOUND
    ${_NAME_UPPER}_LIBRARY
    ${_NAME_UPPER}_LIBRARIES
    ${_NAME_UPPER}_LIBRARY_DIR
    ${_NAME_UPPER}_INCLUDE_DIR
    ${_NAME_UPPER}_INCLUDE_DIRS
    )

#endfunction(check_package NAME INCLUDE_FILE LIBNAME LIBVERSION)
endfunction()

#
#
#
function(require PACKAGE ADD_SEARCH_PATH VERSION)
  message(STATUS "Require package '${PACKAGE}' with version '${VERSION}'")
  string(TOUPPER ${PACKAGE} UPPER_PACKAGE)

  if( ${UPPER_PACKAGE}_FOUND )
    message(STATUS "   Package ${UPPER_PACKAGE} allready loaded")
  endif( ${UPPER_PACKAGE}_FOUND )
  message(STATUS "    Variable ENV{${UPPER_PACKAGE}_HOME} = '$ENV{${UPPER_PACKAGE}_HOME}'")
  message(STATUS "    Variable ${UPPER_PACKAGE}_HOME      = '${${UPPER_PACKAGE}_HOME}'")
  if( NOT ${UPPER_PACKAGE}_HOME)
    message(STATUS "    checking '${UPPER_PACKAGE}_HOME' = '$ENV{${UPPER_PACKAGE}_HOME}' ")
    if( "$ENV{${UPPER_PACKAGE}_HOME}" MATCHES "" )
      set(${UPPER_PACKAGE}_HOME ${ADD_SEARCH_PATH})
      message(STATUS "    Adding additional searchpath: '${${UPPER_PACKAGE}_HOME}'")
      message(STATUS "    Adding additional searchpath: '${ADD_SEARCH_PATH}'")
    endif( "$ENV{${UPPER_PACKAGE}_HOME}" MATCHES "" )
  endif( NOT ${UPPER_PACKAGE}_HOME )

  include (Find${PACKAGE})
  if (${UPPER_PACKAGE}_FOUND)
    include_directories(${${PACKAGE}_INCLUDE_DIR})
    link_libraries(${${PACKAGE}_LIBRARY})
    message(STATUS "    Dependencie '${PACKAGE}' solved.")
  else (${UPPER_PACKAGE}_FOUND)
    message(FATAL "${PACKAGE} could not be found!")
  endif(${UPPER_PACKAGE}_FOUND)
  
endfunction()
