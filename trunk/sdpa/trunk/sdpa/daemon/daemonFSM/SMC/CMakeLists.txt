# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(hdrs DaemonFSM_sm.h DaemonFSM.hpp )

add_state_machine(DAEMON_FSM_CPP DAEMON_FSM_H DaemonFSM)

file(GLOB daemon_fsm_smc_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${DAEMON_FSM_CPP} ${DAEMON_FSM_H} ${daemon_fsm_smc_src})

ADD_LIBRARY(sdpa-daemonFSM_SMC ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/daemonFSM/SMC)
install(TARGETS sdpa-daemonFSM_SMC ARCHIVE DESTINATION lib)
