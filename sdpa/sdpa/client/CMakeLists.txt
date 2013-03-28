# -*- mode: cmake; -*-
find_package(SMC QUIET)
if (SMC_FOUND)
    include_directories(SYSTEM ${SMC_INCLUDE_DIR})
else (SMC_FOUND)
    message (WARNING "SMC could not be found")
endif (SMC_FOUND)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (srcs
  Client.cpp
  )

set (hdrs
  Client.hpp
  ClientActions.hpp
  ClientEvents.hpp
  exceptions.hpp
  types.hpp
  job_info.hpp
  )

#change this later, either use MSM or SMC, when nothing is set
# use by default the generated files
if (SMC_FOUND)
  add_state_machine(CLIENT_FSM_CPP CLIENT_FSM_H ClientFsm)
  set(hdrs ${hdrs} ${CLIENT_FSM_H})
  set(srcs ${srcs} ${CLIENT_FSM_CPP})
else (SMC_FOUND)
  set(hdrs ${hdrs} generated/ClientFsm_sm.h)
  set(srcs ${srcs} generated/ClientFsm_sm.cpp)
endif (SMC_FOUND)

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/sdpa/client
  COMPONENT headers)
install(TARGETS sdpa-client
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
