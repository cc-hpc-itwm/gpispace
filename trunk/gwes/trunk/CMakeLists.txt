project(gwes)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH $ENV{CMAKE_ADDITIONAL_PATH} CACHE PATH "Path where many locally installed libraries can be found")

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

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

## PkgConfig is helpful
include(FindPkgConfig)
if ( PKG_CONFIG_FOUND )
#   pkg_check_modules (CPPUNIT REQUIRED cppunit>=1.12)
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
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${CMAKE_SOURCE_DIR}/gworkflowdl_cpp/include)
include_directories(${CMAKE_SOURCE_DIR}/gwes_cpp/include)
#include_directories(${SMC_INCLUDE_DIR})

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

link_libraries(-Wl,-Bdynamic  -ldl)

include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)

# set GWES_CPP_HOME
set( ENV{GWES_CPP_HOME} ${PROJECT_SOURCE_DIR})
message(STATUS "ENV GWES_CPP_HOME=${PROJECT_SOURCE_DIR}")

# Reflect the package structure
add_subdirectory(gworkflowdl_cpp)
add_subdirectory(gwes_cpp)
enable_testing()
add_subdirectory(tests)

# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
)
