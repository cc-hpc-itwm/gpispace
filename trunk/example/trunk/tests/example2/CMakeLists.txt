
add_executable(example_test_tool2 main.cpp
	test_Example.cpp
	)

#target_link_libraries(example_test_tool2 example)
link_libraries(-Wl,-Bdynamic  -ldl)

add_test(example_test_tool2 ${CMAKE_CURRENT_BINARY_DIR}/example_test_tool2)
