cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)
cmake_policy(SET CMP0011 NEW)

# -*- mode: cmake; -*-
project(libsdpa)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
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
OPTION(USE_SMC "Build the project using the state-machine compiler" YES)

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

# uuid library
include(FindUUID)
if (UUID_FOUND)
  message(STATUS "Found UUID library: Inc:${UUID_INCLUDE_DIR} Lib:${UUID_LIBRARY}")
  include_directories(${UUID_INCLUDE_DIR})
  link_libraries(${UUID_LIBRARY})
else(UUID_FOUND)
  message(FATAL_ERROR "UUID library could not be found but is required")
endif(UUID_FOUND)

# GWES/GWDL
include(FindGwes)
if (GWES_FOUND)
  message(STATUS "Found Gwes: Inc:${GWES_INCLUDE_DIR} Lib:${GWES_LIBRARY}")
  include_directories(${GWES_INCLUDE_DIR})
  link_libraries(${GWES_LIBRARY})
else(GWES_FOUND)
  message(FATAL_ERROR "Gwes could not be found!")
endif (GWES_FOUND)

if (GWDL_FOUND)
  message(STATUS "Found Gwdl: Inc:${GWDL_INCLUDE_DIR} Lib:${GWDL_LIBRARY}")
  include_directories(${GWDL_INCLUDE_DIR})
  link_libraries(${GWDL_LIBRARY})
else(GWDL_FOUND)
  message(FATAL_ERROR "Gwdl could not be found!")
endif (GWDL_FOUND)


# state machine compiler (only when required)
if (USE_SMC)
  include(FindSMC)
  if (SMC_FOUND)
    message(STATUS "Found StateMachineCompiler: ${SMC_JAR}")
    include_directories(${SMC_INCLUDE_DIR})
  else (SMC_FOUND)
    message(FATAL_ERROR "SMC could not be found but is required")
  endif (SMC_FOUND)
  include (SMCMacros)
endif (USE_SMC)

## check for boost
set(BOOST_ROOT "${CMAKE_ADDITIONAL_PATH}" CACHE PATH "Where to find the Boost libraries")
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
include(FindBoost)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
link_libraries(${Boost_LIBRARIES})

# logging
include(FindFhgLog)
if (FHGLOG_FOUND)
  message(STATUS "Found FhgLog: Inc:${FHGLOG_INCLUDE_DIR} Lib:${FHGLOG_LIBRARY}")
  include_directories(${FHGLOG_INCLUDE_DIR})
  link_libraries(${FHGLOG_LIBRARY})
endif(FHGLOG_FOUND)

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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp)
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
add_subdirectory(apps)
enable_testing()
add_subdirectory(tests)


# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
  )

# cpack configuration
include(InstallRequiredSystemLibraries)
set(CPACK_RPM_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_RPM_PACKAGE_VENDOR ${PROJECT_VENDOR})
set(CPACK_RPM_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RPM_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RPM_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
set(CPACK_RPM_PACKAGE_VERSION_MINOR "${V_MINOR}")
set(CPACK_RPM_PACKAGE_VERSION_PATCH "${V_PATCH}")
set(CPACK_RPM_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
if(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #  set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  #  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
  #  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} My Famous Project")
  #  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
  #  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  #  set(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  #  set(CPACK_NSIS_MODIFY_PATH ON)
else(WIN32 AND NOT UNIX)
  set(CPACK_RPM_SOURCE_STRIP_FILES "")
endif(WIN32 AND NOT UNIX)
set(CPACK_BINARY_RPM ON)
include(CPack)
