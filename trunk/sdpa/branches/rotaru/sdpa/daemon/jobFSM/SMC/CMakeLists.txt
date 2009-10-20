# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(hdrs JobFSM_sm.h JobFSM.hpp )

add_state_machine(JOBFSM_CPP JOBFSM_H JobFSM)

file(GLOB job_fsm_smc_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${JOBFSM_CPP} ${JOBFSM_H} ${job_fsm_smc_src})


ADD_LIBRARY(sdpa-jobFSM_SMC ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/SMC)
install(TARGETS sdpa-jobFSM_SMC ARCHIVE DESTINATION lib)
