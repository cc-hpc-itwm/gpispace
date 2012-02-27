# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (srcs
  JobFSM.cpp
  )

set (hdrs
  JobFSM.hpp
  )

# Make sure that the state machine gets incorporated.

if (SMC_FOUND)
	add_state_machine(JOBFSM_CPP JOBFSM_H JobFSM)
	set(hdrs ${hdrs} ${JOBFSM_H})
	set(srcs ${srcs} ${JOBFSM_CPP})
else (SMC_FOUND)
	set(hdrs ${hdrs} generated/JobFSM_sm.h)
	set(srcs ${srcs} generated/JobFSM_sm.cpp)
endif (SMC_FOUND)


ADD_LIBRARY(sdpa-jobFSM_SMC ${srcs})

install(FILES ${hdrs}
  DESTINATION include/sdpa/jobFSM/SMC
  COMPONENT headers)
install(TARGETS sdpa-jobFSM_SMC
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
