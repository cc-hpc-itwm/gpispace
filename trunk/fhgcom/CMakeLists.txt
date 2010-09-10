# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfhgcom)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

include (./version.cmake)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

option(USE_STL_TR1
 "Use tr1 from stl"
 0)

option(NO_IMPLICIT_TEMPLATES
  "Do not let the compiler implicitely instantiate templates"
  OFF
  )

# don't forget to add the configuration option inc libfhglogConfig.cmake.in

set(PROJECT_DESCRIPTION "FHG Communication Framework")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

if (NO_IMPLICIT_TEMPLATES)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-implicit-templates")
  # TODO:  check each  library  etc. and  add a  special  .cpp file  to it  that
  # instantiates all templates...
endif (NO_IMPLICIT_TEMPLATES)

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD_H 1)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options test_exec_monitor)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

# logging
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/fhgcom/fhgcom-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fhgcom/fhgcom-config.hpp DESTINATION include/fhgcom)

add_definitions(-DHAVE_CONFIG_H)

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: default (not-set)")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  string(TOUPPER ${CMAKE_BUILD_TYPE} tmp)
  message(STATUS "additional CXXFLAGS: ${CMAKE_CXX_FLAGS_${tmp}}")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

# add some files to the installation target
INSTALL(FILES README COPYING DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
)

# Reflect the package structure
add_subdirectory(fhgcom)
add_subdirectory(bin)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

# cpack configuration
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR ${PROJECT_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${V_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
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
  set(CPACK_SOURCE_STRIP_FILES "")
endif(WIN32 AND NOT UNIX)
include(CPack)
