# -*- mode: cmake; -*-
project(gwes)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 2)
set(V_MINOR 0)
set(V_PATCH 1)

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH $ENV{CMAKE_ADDITIONAL_PATH} CACHE PATH "Path where many locally installed libraries can be found")

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

# Find doxygen
include(FindDoxygen)
include(FindMscgen)

## find the StateMachineCompiler (SMC)
Include(FindSMC)
Include(SMCMacros)

## find xerces-c
Include(FindXercesC)
IF(NOT XERCESC_FOUND)
  message(FATAL_ERROR "No Xerces-c found, cannot continue.")
ENDIF(NOT XERCESC_FOUND)

## find libxml2
Include(FindXml2)
IF(NOT LIBXML2_FOUND)
  message(FATAL_ERROR "No libxml2 found, cannot continue.")
ENDIF(NOT LIBXML2_FOUND)
include_directories(${LIBXML2_INCLUDE_DIRS})

## find libxslt
Include(FindLibXslt)
IF(NOT LIBXSLT_FOUND)
  message(FATAL_ERROR "No libxslt found, cannot continue.")
ENDIF(NOT LIBXSLT_FOUND)

## find fhglog 
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  ## we are compiling as standalone project. Search for fhglog
  Include(FindFhgLog)
  IF(NOT FHGLOG_FOUND)
    message(FATAL_ERROR "No libfhglog found, cannot continue.")
  ENDIF(NOT FHGLOG_FOUND)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# uuid library
Include(FindUUID)
IF(UUID_FOUND)
  message(STATUS "Found UUID library: Inc:${UUID_INCLUDE_DIR} Lib:${UUID_LIBRARY}")
  include_directories(${UUID_INCLUDE_DIR})
  link_libraries(${UUID_LIBRARY})
ELSE(UUID_FOUND)
  message(FATAL_ERROR "UUID library could not be found but is required")
ENDIF(UUID_FOUND)

## PkgConfig is helpful
include(FindPkgConfig)
if ( PKG_CONFIG_FOUND )
   pkg_check_modules (CPPUNIT REQUIRED cppunit>=1.12)
   message(STATUS "CPPUnit: -I${CPPUNIT_INCLUDE_DIRS} -L${CPPUNIT_LIBRARY_DIRS} -l${CPPUNIT_LIBRARIES}")
else  ( PKG_CONFIG_FOUND )
  message(FATAL "No PKGCONFIG found - hoping for the best.")
  IF(WIN32)
    # search packages by hand
    # CPPunit
    include(FindCppUnit)
  ENDIF(WIN32)
endif ( PKG_CONFIG_FOUND )

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR}/gworkflowdl_cpp/include)
include_directories(${${PROJECT_NAME}_SOURCE_DIR}/gwes_cpp/include)
include_directories(${CMAKE_INCLUDE_PATH})
#include_directories(${SMC_INCLUDE_DIR})
include_directories(${FHGLOG_INCLUDE_DIR})

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

link_libraries(-Wl,-Bdynamic  -ldl)
link_libraries(${FHGLOG_LIBRARY})

# Display and stop on warnings
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_CXX_FLAGS "-Wall -Wno-non-virtual-dtor -Werror -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -W -Wreturn-type -Wshadow")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -pg -fprofile-arcs -ftest-coverage")
  # gprof and gcov support
  set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

### does not work for automatic build! ###
## set GWES_CPP_HOME
#set( ENV{GWES_CPP_HOME} ${PROJECT_SOURCE_DIR})
#message(STATUS "ENV GWES_CPP_HOME=${PROJECT_SOURCE_DIR}")

# Reflect the package structure
add_subdirectory(gworkflowdl_cpp)
add_subdirectory(gwes_cpp)
enable_testing()
add_subdirectory(tests)

# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
)
