# -*- mode: cmake; -*-

## add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES} ${CPPUNIT_STATIC_LDFLAGS})

#add_subdirectory(fhglog)
add_executable(logtest logtest.cpp)
target_link_libraries(logtest fhglog)

