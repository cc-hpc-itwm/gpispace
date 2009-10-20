# -*- mode: cmake; -*-

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_state_machine(CLIENT_FSM_CPP CLIENT_FSM_H ClientFsm)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

set(hdrs ${CLIENT_FSM_H} ${hdrs} )
set(srcs ${CLIENT_FSM_CPP} ${srcs} )

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)
