project (gpi-space)

OPTION (ENABLE_REAL_GPI "Provide api for the *real* GPI" ON)
OPTION (ENABLE_COMPATIBILITY_API "Provide a transitional api compatible with the old fvm-pc" ON)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (QUIET_FIND_PACKAGE QUIET)

if (ENABLE_REAL_GPI)
  find_package (IBVerbs ${QUIET_FIND_PACKAGE})
  if (IBverbs_FOUND)
    include_directories (SYSTEM ${IBverbs_INCLUDE_DIR})

    set (GPI_HOME ${CMAKE_CURRENT_SOURCE_DIR}/ext)
    set (GPI_INCLUDE_DIR ${GPI_HOME}/include)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${GPI_HOME}/lib64/libGPI.a)
    set (GPI_LIBRARIES ${GPI_LIBRARIES} ${IBverbs_LIBRARY})

    # check if we can actually use the GPI version in ext
    set (CMAKE_REQUIRED_INCLUDES ${GPI_INCLUDE_DIR})
    set (CMAKE_REQUIRED_LIBRARIES ${GPI_LIBRARIES})
    include (CheckCSourceRuns)
    CHECK_C_SOURCE_RUNS (
      "
#include <stdio.h>
#include <GPI.h>
int main ()
{
   printf(\"%f\", getVersionGPI());
   return 0;
}"
    REAL_GPI_CHECK)
    if ("${REAL_GPI_CHECK}" EQUAL 1)
      add_definitions (-DENABLE_REAL_GPI=1)
    else()
      message (WARNING "Support for real-GPI requested but not available on your platform!")
      set (ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
    endif()
  else (IBverbs_FOUND)
    message (STATUS "${IBverbs_FOUND}")
    message (WARNING "Support for real-GPI requested but not available on your platform (ibverbs not installed)!")
    set (ENABLE_REAL_GPI OFF CACHE BOOL "Provide api for the *real* GPI" FORCE)
  endif()
endif()

include (CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif()

include (CheckIncludeFile)
check_include_file (sys/types.h HAVE_SYS_TYPES_H)
if (HAVE_SYS_TYPES_H)
else()
  message (FATAL_ERROR "sys/types.h is required for mode_t")
endif()

check_include_file (unistd.h HAVE_UNISTD_H)
if (HAVE_UNISTD_H)
else()
  message (FATAL_ERROR "unistd.h is required for sysconf")
endif()

include (CheckSymbolExists)
check_symbol_exists (_SC_GETPW_R_SIZE_MAX unistd.h HAVE_GETPW_R_SIZE_MAX)
if (HAVE_GETPW_R_SIZE_MAX)
else()
  message (FATAL_ERROR "_SC_GETPW_R_SIZE_MAX doesn't seem to be defined on this machine")
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgLog_INCLUDE_DIR})

find_package (FhgCom REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgCom_INCLUDE_DIR})

find_package (UTIL REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${UTIL_INCLUDE_DIR})

find_package (FhgKernel REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${FhgKernel_INCLUDE_DIR})

find_package (MMGR REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(${MMGR_INCLUDE_DIR})

find_package (LibRT REQUIRED ${QUIET_FIND_PACKAGE})
include_directories(SYSTEM ${LIBRT_INCLUDE_DIR})

find_package (ZMQ ${QUIET_FIND_PACKAGE})
if (ZMQ_FOUND)
  include_directories (SYSTEM ${ZMQ_INCLUDE_DIR})
endif ()

find_package (CZMQ ${QUIET_FIND_PACKAGE})
if (CZMQ_FOUND)
  include_directories (SYSTEM ${CZMQ_INCLUDE_DIR})
endif()

add_subdirectory (gpi-space)
add_subdirectory (bin)

if (ENABLE_COMPATIBILITY_API)
  add_subdirectory (compat)
endif()

if (BUILD_TESTING)
  include(CTest)
  add_subdirectory (tests)
endif()
