# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_state_machine(JOBFSM_CPP JOBFSM_H JobFSM)

file(GLOB srcs *.cpp)
file(GLOB hdrs *.hpp)

# Make sure the state machine gets incorporated.
set(hdrs ${JOBFSM_H}   ${hdrs})
set(srcs ${JOBFSM_CPP}    ${srcs})

ADD_LIBRARY(sdpa-jobFSM_SMC ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/SMC)
install(TARGETS sdpa-jobFSM_SMC ARCHIVE DESTINATION lib)
