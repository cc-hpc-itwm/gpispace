file(GLOB srcs "*/*.cpp")

# add the CPPUNIT stuff
include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})

add_subdirectory(example)
add_subdirectory(example2)

#### Big test for bitten
add_executable(example_test main.cpp ${srcs})

link_libraries(-Wl,-Bdynamic  -ldl)

add_test(example_test ${CMAKE_CURRENT_BINARY_DIR}/example_test)
