# -*- mode: cmake; -*-

file(GLOB srcs "*.cpp")

# add the CPPUNIT stuff
#include_directories(${CPPUNIT_INCLUDE_DIRS})
#link_directories(${CPPUNIT_LIBRARY_DIRS})
#link_libraries(${CPPUNIT_LIBRARIES})


#### Big test for bitten
message(STATUS "linking gwdl-test with srcs=\"${srcs}\"")
add_executable(gwdl_test TestGwdl.cpp ${srcs})
target_link_libraries(gwdl_test gworkflowdl_cpp xerces-c)

add_test(gwdl_test ${CMAKE_CURRENT_BINARY_DIR}/gwdl_test)
