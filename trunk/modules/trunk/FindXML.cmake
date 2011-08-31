# -*- mode: cmake; -*-
# This file defines:
# * XML_FOUND if workflow engine was found
# * XML_PARSER The lib to link to (currently only a static unix lib, not portable)
# * XML_INCLUDE_DIR The path to the include directory

if (NOT XML_FIND_QUIETLY)
  message(STATUS "FindXML check")
endif (NOT XML_FIND_QUIETLY)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_path (XML_INCLUDE_DIR
    NAMES "xml/parse/parser.hpp"
    HINTS ${XML_HOME} ENV XML_HOME
    PATH_SUFFIXES include
    )

  find_program (XML_PARSER
    NAMES "parser"
    HINTS ${XML_HOME} ENV XML_HOME
    PATH_SUFFIXES bin
    )

  if (XML_INCLUDE_DIR)
    set (XML_FOUND TRUE)
    if (NOT XML_FIND_QUIETLY)
      message(STATUS "Found XML headers in ${XML_INCLUDE_DIR}")
      if (XML_PARSER)
	message(STATUS "Found XML parser tool in ${XML_PARSER}")
      endif (XML_PARSER)
    endif (NOT XML_FIND_QUIETLY)
  else (XML_INCLUDE_DIR)
    if (XML_FIND_REQUIRED)
      message (FATAL_ERROR "XML could not be found!")
    endif (XML_FIND_REQUIRED)
  endif (XML_INCLUDE_DIR)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(XML_FOUND true)
  set(XML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/xml;${CMAKE_BINARY_DIR}/xml")
  set(XML_LIBRARY_DIR "")
  set(XML_LIBRARY "")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

