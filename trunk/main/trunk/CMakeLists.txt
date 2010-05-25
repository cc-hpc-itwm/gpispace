# -*- mode: cmake; -*-
project(sdpa_main)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 1)
set(V_MINOR 4)
set(V_PATCH 0)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2009 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

# add a path where some libraries might be stored
set(CMAKE_ADDITIONAL_PATH $ENV{CMAKE_ADDITIONAL_PATH} CACHE PATH "Path where many locally installed libraries can be found")

OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
set(CMAKE_BINARY_PATH ${CMAKE_BINARY_PATH} ${CMAKE_ADDITIONAL_PATH}/bin)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

##########################################################
set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Wno-non-virtual-dtor ")#-Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -W -Wreturn-type")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
# gprof and gcov support
set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")

include(CheckCompiler)
info_compiler()

##########################################################
include(CTest)
message(STATUS "========== Add project fhglog")
add_subdirectory(fhglog)
message(STATUS "========== Add project we")
add_subdirectory(we)
message(STATUS "========== Add project mmgr")
add_subdirectory(mmgr)
message(STATUS "========== Add project fvm-pc")
add_subdirectory(fvm-pc)
message(STATUS "========== Add project seda")
add_subdirectory(seda)
message(STATUS "========== Add project sdpa")
add_subdirectory(sdpa)

include(TargetDoc.cmake OPTIONAL)
