# -*- mode: cmake; -*-

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})

add_subdirectory(seda)

if (WITH_COMM)
  add_subdirectory(sedaComm)
endif (WITH_COMM)
