set(hdrs DaemonFSM_sm.h DaemonFSM.hpp )

add_state_machine(DaemonFSM)

file(GLOB daemon_fsm_smc_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${daemon_fsm_smc_src} DaemonFSM_sm.h DaemonFSM_sm.cpp)

ADD_LIBRARY(sdpa-daemonFSM_SMC ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/daemonFSM/SMC)
install(TARGETS sdpa-daemonFSM_SMC ARCHIVE DESTINATION lib)
