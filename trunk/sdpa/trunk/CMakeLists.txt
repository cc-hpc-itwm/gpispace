# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(sdpa)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

if (V_MAJOR)
else (V_MAJOR)
  set(V_MAJOR 0)
  set(V_MINOR 2)
  set(V_PATCH 0)
endif (V_MAJOR)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2009 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(ENABLE_TYPE_CHECKING "Enable type checking in tokens" YES)
OPTION(USE_STL_TR1 "Use tr1 from the STL not boost." NO)
OPTION(USE_SMC_SC "Build the project using the state-machine compiler" YES)
OPTION(USE_BOOST_SC "Build the project using boost statecharts" NO)
OPTION(USE_GWES "Use GWES" NO)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})


# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

include(FindUTIL)
if (UTIL_FOUND)
  include_directories(${UTIL_INCLUDE_DIR})
else(UTIL_FOUND)
  message(FATAL_ERROR "UTIL could not be found but is required")
endif(UTIL_FOUND)

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
  include_directories(${SEDA_INCLUDE_DIR})
  link_directories(${SEDA_LIBRARY_DIR})
else (SEDA_FOUND)
  message(FATAL_ERROR "SEDA library is required!")
endif(SEDA_FOUND)

# uuid library
include(FindUUID)
if (UUID_FOUND)
  include_directories(${UUID_INCLUDE_DIR})
else(UUID_FOUND)
  message(FATAL_ERROR "UUID library could not be found but is required")
endif(UUID_FOUND)

# we library
include(FindWE)
if (WE_FOUND)
  include_directories(${WE_INCLUDE_DIR})
else(WE_FOUND)
  message(FATAL_ERROR "WE could not be found but is required")
endif(WE_FOUND)

## find zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")

# state machine compiler (only when required)
if (USE_SMC_SC)
  include(FindSMC)
  if (SMC_FOUND)
    message(STATUS "Found StateMachineCompiler: ${SMC_JAR}")
    include_directories(${SMC_INCLUDE_DIR})
  else (SMC_FOUND)
    message(FATAL_ERROR "SMC could not be found but is required")
  endif (SMC_FOUND)
  include (SMCMacros)
endif (USE_SMC_SC)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem serialization program_options)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

if (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 1)
else (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 0)
endif (ENABLE_LOGGING)

# logging
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})
set(SDPA_HAVE_FHGLOG 1)

find_package(MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

find_package(FVM REQUIRED)
message(STATUS "Found FVM: Inc:${FVM_INCLUDE_DIR} Lib:${FVM_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/version.hpp)
add_definitions(-DHAVE_CONFIG_H)

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Reflect the package structure
add_subdirectory(sdpa)
add_subdirectory(apps)
add_subdirectory(bin)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

# add some files to the installation target
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
  )
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp  DESTINATION include/sdpa)
INSTALL(FILES etc/sdpac.rc.example DESTINATION etc/sdpa)

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
