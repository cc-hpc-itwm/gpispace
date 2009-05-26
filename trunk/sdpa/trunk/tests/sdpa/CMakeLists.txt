# -*- mode: cmake; -*-
add_executable(sdpa_test_tool sdpa_test_tool.cpp)
target_link_libraries(sdpa_test_tool sdpa)

add_test(sdpa_test_tool ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_tool)

add_subdirectory(applicationContainer)

