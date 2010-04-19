# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(sdpa)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 2)
set(V_PATCH 0)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2009 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_definitions("-DNDEBUG")
  endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

  if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Wno-non-virtual-dtor")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_DEBUG "-Werror -O0 -g -ggdb -Wreturn-type -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")

    # gprof and gcov support
    set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers -pg")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -fprofile-arcs -ftest-coverage")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

  # TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "-wd383 -wd981")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")

endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(ENABLE_TYPE_CHECKING "Enable type checking in tokens" YES)
OPTION(USE_STL_TR1 "Use tr1 from the STL not boost." NO)
OPTION(USE_SMC_SC "Build the project using the state-machine compiler" YES)
OPTION(USE_BOOST_SC "Build the project using boost statecharts" NO)
OPTION(USE_GWES "Use GWES" NO)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

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

# GWES/GWDL
if( USE_GWES)
include(FindGwes)
# check for gwes/gwdl
if (GWES_FOUND)
  message(STATUS "Found Gwes: Inc:${GWES_INCLUDE_DIR} Lib:${GWES_LIBRARY}")
  include_directories(${GWES_INCLUDE_DIR})
else(GWES_FOUND)
  message(FATAL_ERROR "Gwes could not be found!")
endif (GWES_FOUND)

if (GWDL_FOUND)
  message(STATUS "Found Gwdl: Inc:${GWDL_INCLUDE_DIR} Lib:${GWDL_LIBRARY}")
  include_directories(${GWDL_INCLUDE_DIR})
else(GWDL_FOUND)
  message(FATAL_ERROR "Gwdl could not be found!")
endif (GWDL_FOUND)

## find libxml2
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
message(STATUS "LibXml2: Inc:${LIBXML2_INCLUDE_DIR} Lib:${LIBXML2_LIBRARIES}")

## find libxslt
find_package(LibXslt REQUIRED)
include_directories(${LIBXSLT_INCLUDE_DIR})
message(STATUS "LibXslt: Inc:${LIBXSLT_INCLUDE_DIR} Lib:${LIBXSLT_LIBRARIES}")
endif (USE_GWES)

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
include(FindFhgLog)
if (FHGLOG_FOUND)
  message(STATUS "Found FhgLog: Inc:${FHGLOG_INCLUDE_DIR} Lib:${FHGLOG_LIBRARY}")
  include_directories(${FHGLOG_INCLUDE_DIR})
  set(SDPA_HAVE_FHGLOG 1)
else (FHGLOG_FOUND)
  message(FATAL_ERROR "Sorry, FhgLog is required!")
endif(FHGLOG_FOUND)

# check for Fraunhofer Virtual Machine
message(STATUS "****** FIXME: Checking for Fraunhofer Virtual Machine...")
include(FindFVM OPTIONAL)
if (AP_QUICK_HACK_FVM)
  set (FVM_FOUND "Yes")
  set (FVM_INCLUDE_DIR "/opt/sdpa/fvm/include")
  set (FVM_LIBRARY "/opt/sdpa/fvm/lib/libfvm-pc.a")
  set (HAVE_FVM 1)
endif (AP_QUICK_HACK_FVM)
if (FVM_FOUND)
  # this should actually be specific to the sdpa/modules/ directory (i think)
  include_directories(${FVM_INCLUDE_DIR})
  set (HAVE_FVM 1)
else (FVM_FOUND)
  set (HAVE_FVM 0)
endif (FVM_FOUND)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp)
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
include(CTest)
add_subdirectory(tests)

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
