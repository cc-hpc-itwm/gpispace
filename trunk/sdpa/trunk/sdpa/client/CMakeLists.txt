# -*- mode: cmake; -*-

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
  )

#if (USE_SMC_SC)
  add_state_machine(CLIENT_FSM_CPP CLIENT_FSM_H ClientFsm)
  set(hdrs ${hdrs} ${CLIENT_FSM_H})
  set(srcs ${srcs} ${CLIENT_FSM_CPP})
#endif (USE_SMC_SC)

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs}
  DESTINATION include/sdpa/client
  COMPONENT headers)
install(TARGETS sdpa-client
  ARCHIVE
  DESTINATION lib
  COMPONENT libraries)
