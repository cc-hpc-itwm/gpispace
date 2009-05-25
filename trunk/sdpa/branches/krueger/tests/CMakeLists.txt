# -*- mode: cmake; -*-

file(GLOB srcs "*/t*.cpp */*/*cpp")

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})


#### Big test for bitten
add_executable(sdpa_test main.cpp ${srcs})

link_libraries(-Wl,-Bdynamic  -ldl)

add_test(sdpa_test ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test)

add_subdirectory(sdpa)
