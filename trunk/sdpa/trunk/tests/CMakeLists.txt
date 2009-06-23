# -*- mode: cmake; -*-

file(GLOB srcs "sdpa/*.cpp" "*/*/*.cpp")
message(STATUS "WARNING: this is an ugly workaround for the duplicate main() function problem: removing sdpa_test_tool from the top-level test-suite")
list(REMOVE_ITEM srcs "${CMAKE_CURRENT_SOURCE_DIR}/sdpa/sdpa_test_tool.cpp")

## add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})


##### Big test for bitten
message(STATUS "linking sdpa-test with srcs=\"${srcs}\"")
add_executable(sdpa_test main.cpp ${srcs})
target_link_libraries(sdpa_test sdpa)
target_link_libraries(sdpa_test sdpa-modules)
#
add_test(sdpa_test ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test)
add_subdirectory(sdpa)
