# -*- mode: cmake; -*-

## add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})

add_subdirectory(sdpa)

##### Big test for bitten
#add_executable(sdpa_test main.cpp)
#target_link_libraries(sdpa_test sdpa-test)
#target_link_libraries(sdpa_test sdpa)
#target_link_libraries(sdpa_test sdpa-modules)
#target_link_libraries(sdpa_test sdpa-events)
#target_link_libraries(sdpa_test sdpa-daemon)
#add_test(sdpa_test ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test)
