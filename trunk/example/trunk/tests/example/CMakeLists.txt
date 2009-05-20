file(GLOB example_src ${CMAKE_SOURCE_DIR}/tests/main.cpp "*.cpp")

add_executable(example_test_tool ${example_src})

link_libraries(-Wl,-Bdynamic  -ldl)

add_test(example_test_tool ${CMAKE_CURRENT_BINARY_DIR}/example_test_tool)
