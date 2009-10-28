# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libseda)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 1)
set(V_MINOR 2)
set(V_PATCH 0)

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

OPTION(WITH_COMM
  "Build communication library"
  1)

OPTION(USE_STL_TR1 
 "Use tr1 from stl"
 1)

OPTION(ENABLE_UNIT_TESTS
  "Build and perform unit tests"
  YES)


# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})

# Definitions for different compilations.
# Debug is the default development mode. Warn as much as possible, but
# CPPUnit raises some errors about virtual functions - suppress them.
if (WIN32)
  # make windows happy
else(WIN32)
  if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror")

    # Releases are made with the release build. Optimize code.
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
      add_definitions("-DNDEBUG")
    endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

    if (NOT ENABLE_LOGGING)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
    endif (NOT ENABLE_LOGGING)

    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wreturn-type -fno-omit-frame-pointer -Wnon-virtual-dtor")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused -Wunused-variable -Wunused-function")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
    # gprof and gcov support
    set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")
  endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
endif(WIN32)

## add doxygen support
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc.cmake OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

## PkgConfig is helpful
include(FindPkgConfig)
if ( PKG_CONFIG_FOUND )
  if (ENABLE_UNIT_TESTS)
    pkg_check_modules (CPPUNIT cppunit>=1.12)
    message(STATUS "CPPUnit: ${CPPUNIT_CFLAGS} ${CPPUNIT_LDFLAGS} STATIC=${CPPUNIT_STATIC_LDFLAGS}")
  endif (ENABLE_UNIT_TESTS)

else  ( PKG_CONFIG_FOUND )
  message("No PKGCONFIG found - hoping for the best.")
endif ( PKG_CONFIG_FOUND )

## logging
if (ENABLE_LOGGING)
  message(STATUS "Project: '${PROJECT_NAME}'")
  set(${UPPER_PROJECT_NAME}_ENABLE_LOGGING ${ENABLE_LOGGING})

  if (WITH_LOG4CPP)
    set (WITH_FHGLOG NO)
    message(STATUS "Building with log4cpp...")

    include (FindLogging)
    if (LOG4CPP_FOUND)
      include_directories(${LOG4CPP_INCLUDE_DIR})
      link_libraries(${LOG4CPP_LIBRARY})
      set(${UPPER_PROJECT_NAME}_HAVE_LOG4CPP 1)
    else (LOG4CPP_FOUND)
      message(FATAL "Log4Cpp could not be found!")
    endif(LOG4CPP_FOUND)
  endif(WITH_LOG4CPP)

  if (WITH_FHGLOG)
    message(STATUS "Building with fhglog...")
    include(FindFhgLog)
    if (FHGLOG_FOUND)
      message(STATUS "Found FhgLog: Inc:${FHGLOG_INCLUDE_DIR} Lib:${FHGLOG_LIBRARY}")
      include_directories(${FHGLOG_INCLUDE_DIR})
      link_libraries(${FHGLOG_LIBRARY})
      set(${UPPER_PROJECT_NAME}_HAVE_FHGLOG 1)
    else (FHGLOG_FOUND)
      message(FATAL "could not find FhgLog")
    endif (FHGLOG_FOUND)
  endif(WITH_FHGLOG)
endif (ENABLE_LOGGING)

## check for threads
Include(FindThreads)

if (CMAKE_USE_PTHREADS_INIT)
  set (HAVE_PTHREAD_H 1)
endif (CMAKE_USE_PTHREADS_INIT)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
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
link_directories(${Boost_LIBRARY_DIRS})
link_libraries(${Boost_LIBRARIES})

if (WITH_COMM)
  # google protocol buffers
  include(FindProtocolBuffers)
  if (PB_FOUND)
      message(STATUS "Found Protocol Buffers: ${PB_PROTOC_CMD}")
      include_directories(${PB_INCLUDE_DIR})
      link_libraries(${PB_LIBRARY})
  endif (PB_FOUND)
  include(PBMacros)
endif (WITH_COMM)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/seda/seda-config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libseda.pc.in   ${CMAKE_CURRENT_BINARY_DIR}/libseda.pc @ONLY)

# this file is installed and contains all necessary information about the installed libseda,
# it also loads the file with the exported targets
configure_file(${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

# Make sure all subdirectories include correctly
link_directories(${${PROJECT_NAME}_BINARY_DIR})
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries(-lpthread)
endif(CMAKE_USE_PTHREADS_INIT)

include(CTest)

# Reflect the package structure
add_subdirectory(seda)
if (WITH_COMM)
  add_subdirectory(seda/comm)
endif (WITH_COMM)

if (ENABLE_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif (ENABLE_UNIT_TESTS)

# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
  )

# install the config
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/seda/seda-config.hpp
  DESTINATION include/seda
)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  install(EXPORT ${PROJECT_NAME}Exports DESTINATION lib/shared
    FILE ${PROJECT_NAME}Targets.cmake)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/shared )

# install pkgconfig file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libseda.pc
  DESTINATION lib/pkgconfig
)

# CPACK packaging
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SEDA C++ library")
SET(CPACK_PACKAGE_VENDOR "Fraunhofer ITWM")
SET(CPACK_PACKAGE_CONTACT "petry@itwm.fhg.de")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR ${V_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${V_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${V_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

SET(CPACK_GENERATOR "DEB;RPM;STGZ;TGZ;TBZ2")

IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} SEDA C++ library")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.xenbee.net")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.xenbee.net")
  SET(CPACK_NSIS_CONTACT "petry@itwm.fhg.de")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
INCLUDE(CPack)
