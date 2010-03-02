# -*- mode: cmake; -*-
project(libsdpa)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

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

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD_H 1)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

## PkgConfig is helpful
include(FindPkgConfig)
if ( PKG_CONFIG_FOUND )
   pkg_check_modules (CPPUNIT REQUIRED cppunit>=1.12)
   message(STATUS "CPPUnit: -I${CPPUNIT_INCLUDE_DIRS} -L${CPPUNIT_LIBRARY_DIRS} -l${CPPUNIT_LIBRARIES}")

else  ( PKG_CONFIG_FOUND )
  message(FATAL "No PKGCONFIG found - hoping for the best.")
    # search packages by hand
    # CPPunit
    ##include(FindCppUnit)
endif ( PKG_CONFIG_FOUND )

# seda library
include(FindSEDA)
# check for seda
if (SEDA_FOUND)
    include_directories(${SEDA_INCLUDE_DIRS})
    link_directories(${SEDA_LIBRARY_DIRS})
    link_libraries(${SEDA_LIBRARIES})
endif(SEDA_FOUND)

# google protocol buffers
include(FindProtocolBuffers)
if (PB_FOUND)
    message(STATUS "Found Protocol Buffers: ${PB_PROTOC_CMD}, Inc:${PB_INCLUDE_DIR} Lib:${PB_LIBRARY}")
    include_directories(${PB_INCLUDE_DIR})
    link_libraries(${PB_LIBRARY})
endif (PB_FOUND)
include(PBMacros)

# logging
if (ENABLE_LOGGING)
    set(ENABLE_LOGGING 1)
    include(FindLogging)

    if (LOG4CPP_FOUND)
        set(HAVE_LOG4CPP 1)
        include_directories(${LOG4CPP_INCLUDE_DIRS})
	link_directories(${LOG4CPP_LIBRARY_DIRS})
        link_libraries(${LOG4CPP_LIBRARIES})
    else(LOG4CPP_FOUND)
        message(FATAL_ERROR "Logging was enabled but log4cpp could not be found!")
    endif(LOG4CPP_FOUND)

else (ENABLE_LOGGING)
    set(ENABLE_LOGGING 0)
endif (ENABLE_LOGGING)

## check for boost
set(BOOST_ROOT "${CMAKE_ADDITIONAL_PATH}" CACHE PATH "Where to find the Boost libraries")
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
find_package(Boost 1.36 REQUIRED COMPONENTS thread)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in    ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions(-DHAVE_CONFIG_H)

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})
include_directories(${Boost_INCLUDE_DIRS})

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})

include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)

# Reflect the package structure
add_subdirectory(sdpa)
add_subdirectory(daemons)
enable_testing()
add_subdirectory(tests)


# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
  )
