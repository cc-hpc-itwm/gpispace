# -*- mode: cmake; -*-
include_directories(${SMC_INCLUDE_DIR})

file(GLOB srcs "*/t*.cpp" "*/*/*cpp")

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})


#### Big test for bitten
message(STATUS "linking sdpa-test with srcs=\"${srcs}\"")
add_executable(sdpa_test main.cpp ${srcs})
target_link_libraries(sdpa_test sdpa jobFSM orchFSM sdpa-events)
target_link_libraries(sdpa_test)

add_test(sdpa_test ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test)
add_subdirectory(sdpa)
