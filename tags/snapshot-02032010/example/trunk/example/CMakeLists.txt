# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_state_machine(EXAMPLE_FSM_CPP EXAMPLE_FSM_H example)


set(example_src ${EXAMPLE_FSM_CPP} ${EXAMPLE_FSM_H} example.cpp
  )

ADD_LIBRARY(example ${example_src})
