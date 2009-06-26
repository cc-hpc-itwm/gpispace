set(hdrs JobFSM_sm.h JobFSM.hpp )

add_state_machine(JobFSM)

file(GLOB job_fsm_smc_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${job_fsm_smc_src} JobFSM_sm.h JobFSM_sm.cpp)

ADD_LIBRARY(sdpa-jobFSM_SMC ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/SMC)
install(TARGETS sdpa-jobFSM_SMC ARCHIVE DESTINATION lib)
