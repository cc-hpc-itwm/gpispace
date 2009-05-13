set(hdrs JobFSM_sm.h JobFSM.hpp )

include_directories(${SMC_INCLUDE_DIR})

add_state_machine(JobFSM)

file(GLOB fsm_src *.cpp *.hpp)

# Make sure the state machine gets incorporated.
set(fsm_src ${fsm_src} JobFSM_sm.h JobFSM_sm.cpp )

ADD_LIBRARY(sdpa-fsm ${fsm_src})

install(FILES ${hdrs} DESTINATION include/sdpa/fsm)
install(TARGETS sdpa-fsm ARCHIVE DESTINATION lib)
