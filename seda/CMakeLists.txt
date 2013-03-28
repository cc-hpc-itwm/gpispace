# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(seda)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

if (V_MAJOR)
else (V_MAJOR)
  set(V_MAJOR 1)
  set(V_MINOR 4)
  set(V_PATCH 0)
endif (V_MAJOR)

###################################################################################################
# No user settings below
###################################################################################################
# don't forget to add the configuration option inc libsedaConfig.cmake.in

OPTION(ENABLE_LOGGING
  "Build the project with logging enabled"
  1)

OPTION(WITH_FHGLOG
  "Build the project with fhglog"
  1)

OPTION(WITH_LOG4CPP
  "Build the project with log4cpp"
  0)

OPTION(ENABLE_SEDA_COMM_LIB
  "Build communication library"
  OFF)

OPTION(USE_STL_TR1
 "Use tr1 from stl"
 0)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (UseCodeCoverage)
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

## add doxygen support
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc.cmake OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

## logging
if (ENABLE_LOGGING)
  message(STATUS "Project: '${PROJECT_NAME}'")
  set(${UPPER_PROJECT_NAME}_ENABLE_LOGGING ${ENABLE_LOGGING})

    message(STATUS "Building with fhglog...")
	find_package (FhgLog REQUIRED)
    include_directories(${FhgLog_INCLUDE_DIR})
    set(${UPPER_PROJECT_NAME}_HAVE_FHGLOG 1)
endif (ENABLE_LOGGING)

## check for threads
Include(FindThreads)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

## find zlib
if( NOT WIN32)
  find_package(ZLIB REQUIRED)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
  message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")
endif( NOT WIN32)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/seda/seda-config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libseda.pc.in   ${CMAKE_CURRENT_BINARY_DIR}/libseda.pc @ONLY)

# this file is installed and contains all necessary information about the installed libseda,
# it also loads the file with the exported targets
configure_file(${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

# Make sure all subdirectories include correctly

# Reflect the package structure
add_subdirectory(seda)
if (ENABLE_SEDA_COMM_LIB)
  add_subdirectory(seda/comm)
endif ()

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

# install the config
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/seda/seda-config.hpp
  DESTINATION include/seda
  COMPONENT headers
)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  install(EXPORT ${PROJECT_NAME}Exports
    DESTINATION lib/shared
    FILE ${PROJECT_NAME}Targets.cmake
    COMPONENT libraries)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/shared
  COMPONENT libraries)

# install pkgconfig file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libseda.pc
  DESTINATION lib/pkgconfig
  COMPONENT libraries
)
