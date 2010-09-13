# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_state_machine(JOBFSM_CPP JOBFSM_H JobFSM)

set (srcs
  JobFSM.cpp
  )

set (hdrs
  JobFSM.hpp
  )

# Make sure that the state machine gets incorporated.
set(hdrs ${hdrs} ${JOBFSM_H})
set(srcs ${srcs} ${JOBFSM_CPP})

ADD_LIBRARY(sdpa-jobFSM_SMC ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM/SMC)
install(TARGETS sdpa-jobFSM_SMC ARCHIVE DESTINATION lib)
