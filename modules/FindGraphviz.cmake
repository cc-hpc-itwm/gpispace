# bernd.loerwald@itwm.fraunhofer.de

# GRAPHVIZ_FOUND - system has Graphviz
# GRAPHVIZ_INCLUDE_DIR - the Graphviz include directory
# GRAPHVIZ_CDT_LIBRARY, GRAPHVIZ_GVC_LIBRARY,
# GRAPHVIZ_GRAPH_LIBRARY, GRAPHVIZ_PATHPLAN_LIBRARY
# GRAPHVIZ_LIBRARIES - all of the above for lazy people. (me)

set (_graphviz_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
if (WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
else()
  set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

if (NOT $ENV{GRAPHVIZ_HOME} STREQUAL "")
  set (_graphviz_INCLUDE_SEARCH_DIRS $ENV{GRAPHVIZ_HOME}/include ${_graphviz_INCLUDE_SEARCH_DIRS})
  set (_graphviz_LIBRARIES_SEARCH_DIRS $ENV{GRAPHVIZ_HOME}/lib ${_graphviz_INCLUDE_SEARCH_DIRS})
endif()

if (GRAPHVIZ_HOME)
  set (_graphviz_INCLUDE_SEARCH_DIRS ${GRAPHVIZ_HOME}/include ${_graphviz_INCLUDE_SEARCH_DIRS})
  set (_graphviz_LIBRARIES_SEARCH_DIRS ${GRAPHVIZ_HOME}/lib ${_graphviz_LIBRARIES_SEARCH_DIRS})
endif()

# search include directory
find_path (GRAPHVIZ_INCLUDE_DIR
  NAMES gvc.h
  HINTS ${_graphviz_INCLUDE_SEARCH_DIRS}
  PATH_SUFFIXES graphviz
  )

foreach (comp ${Graphviz_FIND_COMPONENTS})
  find_library ( GRAPHVIZ_${comp}_LIBRARY
                 NAMES ${comp}
                 HINTS ${_graphviz_LIBRARIES_SEARCH_DIRS}
                 PATH_SUFFIXES graphviz
               )

  if (GRAPHVIZ_${comp}_LIBRARY)
    set (Graphviz_${comp}_FOUND YES)
    set (GRAPHVIZ_LIBRARIES ${GRAPHVIZ_${comp}_LIBRARY} ${GRAPHVIZ_LIBRARIES})
  endif()

  mark_as_advanced (GRAPHVIZ_${comp}_LIBRARY Graphviz_${comp}_FOUND)
endforeach()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args ( Graphviz
                                    REQUIRED_VARS GRAPHVIZ_INCLUDE_DIR
                                    HANDLE_COMPONENTS
                                  )

mark_as_advanced (GRAPHVIZ_FOUND GRAPHVIZ_INCLUDE_DIR GRAPHVIZ_LIBRARIES)

set (CMAKE_FIND_LIBRARY_SUFFIXES ${_graphviz_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
unset (_graphviz_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES)
unset (_graphviz_INCLUDE_SEARCH_DIRS)
unset (_graphviz_LIBRARY_SEARCH_DIRS)
