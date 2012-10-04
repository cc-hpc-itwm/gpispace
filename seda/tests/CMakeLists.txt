# -*- mode: cmake; -*-

find_package (CppUnit)
if (CppUnit_FOUND)
  include_directories(${CppUnit_INCLUDE_DIR})

  add_subdirectory(seda)

  if (WITH_COMM)
    add_subdirectory(sedaComm)
  endif (WITH_COMM)
else ()
  message (WARNING "Could not find CppUnit, disabling tests")
endif ()
