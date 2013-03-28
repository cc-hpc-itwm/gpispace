# -*- mode: cmake; -*-

find_package (CppUnit QUIET)
if (CppUnit_FOUND)
  include_directories(SYSTEM ${CppUnit_INCLUDE_DIR})

  add_subdirectory(seda)

  if (ENABLE_SEDA_COMM_LIB)
    add_subdirectory(sedaComm)
  endif ()
else ()
  message (WARNING "Could not find CppUnit, disabling tests")
endif ()
