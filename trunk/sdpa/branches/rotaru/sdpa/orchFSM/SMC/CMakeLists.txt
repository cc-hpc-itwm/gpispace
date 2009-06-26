set(hdrs OrchFSM_sm.h OrchFSM.hpp )

include_directories(${SMC_INCLUDE_DIR})
include_directories(.)

add_state_machine(OrchFSM)

file(GLOB orch_fsm_smc_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${orch_fsm_smc_src} OrchFSM_sm.h OrchFSM_sm.cpp ../OrchFSMInterfaceImpl.cpp)

ADD_LIBRARY(sdpa-orchFSM_SMC ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/orchFSM/SMC)
install(TARGETS sdpa-orchFSM_SMC ARCHIVE DESTINATION lib)
