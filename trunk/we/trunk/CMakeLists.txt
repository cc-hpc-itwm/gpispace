# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libwe)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

# have the full monty in makefiles
set(CMAKE_VERBOSE_MAKEFILE true)

set(V_MAJOR 0)
set(V_MINOR 1)
set(V_PATCH 0)

set(PROJECT_DESCRIPTION "Workflow Engine Header only library")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
if (${CMAKE_ADDITIONAL_PATH} NOT STREQUAL "")
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_ADDITIONAL_PATH}/include)
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_ADDITIONAL_PATH}/lib)
  set(CMAKE_BINARY_PATH ${CMAKE_BINARY_PATH} ${CMAKE_ADDITIONAL_PATH}/bin)
endif (${CMAKE_ADDITIONAL_PATH} NOT STREQUAL "")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    if (${CMAKE_BUILD_TYPE} MATCHES "Release")
      add_definitions("-DNDEBUG")
    endif (${CMAKE_BUILD_TYPE} MATCHES "Release")

    set(PROFILE_FLAGS "-pg -fprofile-arcs -ftest-coverage")

    set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wmissing-include-dirs -Wswitch-default -Wno-pragmas -Wfloat-equal -Wenum-compare -Wmissing-declarations -Wredundant-decls")
    add_definitions("-D_FORTIFY_SOURCE=2")

    set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")

    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb -fno-omit-frame-pointer")
    if (${CMAKE_CXX_COMPILER_MAJOR} GREATER 3)
      if (${CMAKE_CXX_COMPILER_MINOR} GREATER 3)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic -Wreturn-type -Wshadow")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-variable -Wunused-parameter")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wunused-function -Wunused")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
        if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
          message(STATUS "Additional warnings for Debug-Build activated (gcc >= 4.3 detected)")
        endif (${CMAKE_BUILD_TYPE} MATCHES "Debug")
      elseif (${CMAKE_CXX_COMPILER_MINOR} GREATER 0)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wreturn-type")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Woverloaded-virtual -Wno-system-headers")
      endif (${CMAKE_CXX_COMPILER_MINOR} GREATER 3)
    endif (${CMAKE_CXX_COMPILER_MAJOR} GREATER 3)

    # gprof and gcov support
    set(CMAKE_CXX_FLAGS_PROFILE "-O0 -g -ggdb -Wreturn-type -Woverloaded-virtual")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} -Wno-system-headers")
    set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${PROFILE_FLAGS}")
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")

  # TODO: we need to check the compiler here, gcc does not know about those flags, is this The Right Thing To Do (TM)?
  if (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "-wd383 -wd981")
    message(STATUS "compiler: ${CMAKE_CXX_COMPILER_MAJOR}.${CMAKE_CXX_COMPILER_MINOR}")
    if (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
      message(STATUS "Warning: adding __aligned__=ignored to the list of definitions")
      add_definitions("-D__aligned__=ignored")
    endif (${CMAKE_CXX_COMPILER_MAJOR} GREATER 9)
  endif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")

endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

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
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/we/we-config.hpp DESTINATION include/we)
add_definitions(-DHAVE_CONFIG_H)

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
INSTALL(FILES INSTALL README COPYING ChangeLog NEWS DESTINATION
  share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}
)

# CTest
include(CTest)

# Reflect the package structure
add_subdirectory(we)
add_subdirectory(tests)
add_subdirectory(kdm)

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
