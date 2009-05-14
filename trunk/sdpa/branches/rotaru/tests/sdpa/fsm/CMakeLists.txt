include_directories(${CPPUNIT_INCLUDE_DIRS})
link_directories(${CPPUNIT_LIBRARY_DIRS})
link_libraries(${CPPUNIT_LIBRARIES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(fsm_test_src main.cpp FsmTest.cpp FsmTest.hpp )

add_executable(fsm_test ${fsm_test_src})

add_test(fsm_test ${CMAKE_CURRENT_BINARY_DIR}/fsm_test)
target_link_libraries(fsm_test sdpa-messages sdpa-events ${PB_LIBRARY})
