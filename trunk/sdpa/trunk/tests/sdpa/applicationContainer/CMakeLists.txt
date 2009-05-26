file(GLOB srcs "*.cpp")
file(GLOB allsrcs ${CMAKE_SOURCE_DIR}/tests/main.cpp ${srcs})

add_executable(container_test_tool ${allsrcs})
target_link_libraries(container_test_tool sdpa)
target_link_libraries(container_test_tool modulcontainer)

link_libraries(-Wl,-Bdynamic  -ldl)

# add_test(container_test_tool  ${CMAKE_CURRENT_BINARY_DIR}/applicationContainer)
# add_test(container_test_tool ${CMAKE_CURRENT_BINARY_DIR})
