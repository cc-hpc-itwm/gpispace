# -*- mode: cmake; -*-

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES} dl)

#file(GLOB dirs "${CMAKE_CURRENT_SOURCE_DIR}/*")
add_subdirectory("seda")

if (ENABLE_COMM_LIB)
  add_subdirectory("sedaComm")
endif (ENABLE_COMM_LIB)
