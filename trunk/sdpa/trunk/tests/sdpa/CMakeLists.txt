# -*- mode: cmake; -*-
add_state_machine(PerformanceTest)
add_executable(sdpa_test_tool sdpa_test_tool.cpp)
target_link_libraries(sdpa_test_tool sdpa)

add_test(sdpa_test_tool ${CMAKE_CURRENT_BINARY_DIR}/sdpa_test_tool)

file(GLOB testSrc "test_*.cpp")
file(GLOB allsrcs ${CMAKE_SOURCE_DIR}/tests/main.cpp ${srcs})
list(APPEND allsrcs "${CMAKE_CURRENT_SOURCE_DIR}/PerformanceTest_sm.h")

add_executable(sdpa_test_suite ${allsrcs})
target_link_libraries(sdpa_test_suite sdpa)
target_link_libraries(sdpa_test_suite sdpa-modules)
target_link_libraries(sdpa_test_suite dl)

# test module
add_library(example-mod SHARED ExampleModule.cpp)
#link_libraries(-Wl,-Bdynamic  -ldl)
