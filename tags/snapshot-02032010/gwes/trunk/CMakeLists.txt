# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(gwes)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 2)
set(V_MINOR 0)
set(V_PATCH 3)

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH $ENV{CMAKE_ADDITIONAL_PATH} CACHE PATH "Path where many locally installed libraries can be found")

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR}/gworkflowdl_cpp/include)
include_directories(${${PROJECT_NAME}_SOURCE_DIR}/gwes_cpp/include)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# Find doxygen
include(FindDoxygen)
include(FindMscgen)

## find the StateMachineCompiler (SMC)
#include(FindSMC)
#include(SMCMacros)

## find libxml2
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
message(STATUS "LibXml2: Inc:${LIBXML2_INCLUDE_DIR} Lib:${LIBXML2_LIBRARIES}")

## find libxslt
find_package(LibXslt REQUIRED)
include_directories(${LIBXSLT_INCLUDE_DIR})
message(STATUS "LibXslt: Inc:${LIBXSLT_INCLUDE_DIR} Lib:${LIBXSLT_LIBRARIES}")

## find zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")

## find fhglog 
include(FindFhgLog)
if (FHGLOG_FOUND)
  message(STATUS "Found FhgLog: Inc:${FHGLOG_INCLUDE_DIR} Lib:${FHGLOG_LIBRARY}")
  include_directories(${FHGLOG_INCLUDE_DIR})
endif(FHGLOG_FOUND)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

# uuid library
Include(FindUUID)
IF(UUID_FOUND)
  message(STATUS "Found UUID library: Inc:${UUID_INCLUDE_DIR} Lib:${UUID_LIBRARY}")
  include_directories(${UUID_INCLUDE_DIR})
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

#include_directories(${SMC_INCLUDE_DIR})

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

#link_libraries(-Wl,-Bdynamic  -ldl)

# Display and stop on warnings
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(CMAKE_CXX_FLAGS "-Wall -Wno-non-virtual-dtor -Werror -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
#  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -W -Wreturn-type -Wshadow")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -W -Wreturn-type")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
  # gprof and gcov support
  set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
  set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

### does not work for automatic build! ###
### not supported for ctest yet. will need cmake 2.8
## set GWES_CPP_HOME
#set( ENV{GWES_CPP_HOME} ${PROJECT_SOURCE_DIR})
#message(STATUS "ENV GWES_CPP_HOME=$ENV{GWES_CPP_HOME}")

# Reflect the package structure
add_subdirectory(gworkflowdl_cpp)
add_subdirectory(gwes_cpp)
enable_testing()
add_subdirectory(tests)

# add some files to the installation target
INSTALL(FILES README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
)
