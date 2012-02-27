# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (srcs
  DaemonFSM.cpp
  )

set (hdrs
  DaemonFSM.hpp
  )

if (SMC_FOUND)
# Make sure the state machine gets incorporated.
	add_state_machine(DAEMON_FSM_CPP DAEMON_FSM_H DaemonFSM)
	set(hdrs ${hdrs} ${DAEMON_FSM_H})
	set(srcs ${srcs} ${DAEMON_FSM_CPP})
else (SMC_FOUND)
# simply, use the generated files
	set(hdrs ${hdrs} generated/DaemonFSM_sm.h)
	set(srcs ${srcs} generated/DaemonFSM_sm.cpp)
endif (SMC_FOUND)

ADD_LIBRARY(sdpa-daemonFSM_SMC ${srcs})
target_link_libraries(sdpa-daemonFSM_SMC sdpa-events)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemonFSM/SMC
  COMPONENT headers)
install(TARGETS sdpa-daemonFSM_SMC
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
