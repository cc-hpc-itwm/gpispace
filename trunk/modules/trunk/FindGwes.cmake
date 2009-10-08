# -*- mode: cmake; -*-
# locates the Gwes Workflow Engine
# This file defines:
# * GWES_FOUND if gwes was was found
# * GWES_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWES_INCLUDE_DIR
#
# * GWDL_FOUND if gwdl was was found
# * GWDL_LIBRARY The lib to link to (currently only a static unix lib) 
# * GWDL_INCLUDE_DIR

message(STATUS "FindGwes check")
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FindPackageHelper)
  check_package(GWES gwes/GWES.h gwes_cpp 1.0)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(GWES_FOUND true)
  set(GWES_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gwes/gwes_cpp/include;${CMAKE_BINARY_DIR}/gwes")
  set(GWES_LIBRARY_DIR "")
  set(GWES_LIBRARY gwes_cpp)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})


##
##
## GWDL stuff
##
##
message(STATUS "FindGwdl check")
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(FindPackageHelper)
  check_package(GWDL gwdl/Workflow.h gworkflowdl_cpp 1.0)
else(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(GWDL_FOUND true)
  set(GWDL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/gwes/gworkflowdl_cpp/include;${CMAKE_BINARY_DIR}/gwdl")
  set(GWDL_LIBRARY_DIR "")
  set(GWDL_LIBRARY gworkflowdl_cpp)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

