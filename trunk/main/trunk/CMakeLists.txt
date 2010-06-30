# -*- mode: cmake; -*-
project(sdpa_main)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

include (./version.cmake)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH $ENV{CMAKE_ADDITIONAL_PATH} CACHE PATH "Path where many locally installed libraries can be found")

OPTION(ENABLE_TESTS "Build with tests" YES)
OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_BINARY_PATH ${CMAKE_BINARY_PATH} ${CMAKE_ADDITIONAL_PATH}/bin)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

##########################################################
include(CheckCompiler)
info_compiler()
include (CompilerFlags)

# cpack configuration
#include(InstallRequiredSystemLibraries)
#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
#set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
     set(CPACK_PACKAGE_NAME "sdpa-dbg")
else (${CMAKE_BUILD_TYPE} MATCHES "Debug")
     set(CPACK_PACKAGE_NAME "sdpa")
endif (${CMAKE_BUILD_TYPE} MATCHES "Debug")

set(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${V_PATCH}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_RPM_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#include(InstallRequiredSystemLibraries)
#set(CPACK_RPM_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
#set(CPACK_RPM_PACKAGE_VENDOR ${PROJECT_VENDOR})
#set(CPACK_RPM_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
#set(CPACK_RPM_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
#set(CPACK_RPM_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
#set(CPACK_RPM_PACKAGE_VERSION_MINOR "${V_MINOR}")
#set(CPACK_RPM_PACKAGE_VERSION_PATCH "${V_PATCH}")
#set(CPACK_RPM_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

#if(WIN32 AND NOT UNIX)
#else(WIN32 AND NOT UNIX)
#  set(CPACK_RPM_SOURCE_STRIP_FILES "")
#endif(WIN32 AND NOT UNIX)
#set(CPACK_BINARY_RPM ON)
#include(CPack)

##########################################################
include(CTest)
message(STATUS "========== Add project fhglog")
add_subdirectory(fhglog)
message(STATUS "========== Add project we")
add_subdirectory(we)
message(STATUS "========== Add project fvm-pc")
add_subdirectory(fvm-pc)
message(STATUS "========== Add project mmgr")
add_subdirectory(mmgr)
message(STATUS "========== Add project seda")
add_subdirectory(seda)
message(STATUS "========== Add project sdpa")
add_subdirectory(sdpa)

include(TargetDoc.cmake OPTIONAL)
