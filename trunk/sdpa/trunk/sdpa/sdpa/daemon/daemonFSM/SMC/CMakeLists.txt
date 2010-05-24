# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_state_machine(DAEMON_FSM_CPP DAEMON_FSM_H DaemonFSM)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.hpp)

# Make sure the state machine gets incorporated.
set(hdrs ${DAEMON_FSM_H}   ${hdrs})
set(srcs ${DAEMON_FSM_CPP} ${srcs})

ADD_LIBRARY(sdpa-daemonFSM_SMC ${srcs})
target_link_libraries(sdpa-daemonFSM_SMC sdpa-events)

install(FILES ${hdrs} DESTINATION include/sdpa/daemonFSM/SMC)
install(TARGETS sdpa-daemonFSM_SMC ARCHIVE DESTINATION lib)
