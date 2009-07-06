file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

add_state_machine(ClientFsm)
list(FIND srcs ${ClientFsm_C} contains_fsm_c)
if(${contains_fsm_c} EQUAL -1)
  list(APPEND hdrs ${ClientFsm_H})
  list(APPEND srcs ${ClientFsm_C})
else(${contains_fsm_c} EQUAL -1)
endif(${contains_fsm_c} EQUAL -1)

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)
