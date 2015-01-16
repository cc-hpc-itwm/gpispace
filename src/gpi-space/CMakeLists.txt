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

include_directories(${FhgLog_INCLUDE_DIR})

find_package (LibRT REQUIRED QUIET)
include_directories(SYSTEM ${LIBRT_INCLUDE_DIR})

set (hdrs
  types.hpp
  exception.hpp
  )

add_subdirectory(gpi)
add_subdirectory(pc)
add_subdirectory (tests)
