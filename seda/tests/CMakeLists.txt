# -*- mode: cmake; -*-

find_package (CppUnit REQUIRED)
include_directories(${CppUnit_INCLUDE_DIR})

add_subdirectory(seda)

if (WITH_COMM)
  add_subdirectory(sedaComm)
endif (WITH_COMM)
