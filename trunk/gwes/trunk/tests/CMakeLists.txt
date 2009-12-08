# -*- mode: cmake; -*-

file(GLOB gwdlsrcs "gwdl/*.cpp")
file(GLOB gwessrcs "gwes/*.cpp")

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})

#### Big test for bitten
# test gwdl
add_executable(gwdl_test ${gwdlsrcs})
target_link_libraries(gwdl_test gworkflowdl_cpp)
target_link_libraries(gwdl_test ${CPPUNIT_LIBRARIES})
add_test(gwdl_test ${CMAKE_CURRENT_BINARY_DIR}/gwdl_test)

# test gwes
add_executable(gwes_test ${gwessrcs})
target_link_libraries(gwes_test gwes_cpp gworkflowdl_cpp)
target_link_libraries(gwes_test ${CPPUNIT_LIBRARIES})
add_test(gwes_test ${CMAKE_CURRENT_BINARY_DIR}/gwes_test)
