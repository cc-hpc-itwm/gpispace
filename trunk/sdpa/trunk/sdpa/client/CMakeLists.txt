file(GLOB hdrs *.hpp)
file(GLOB srcs *.cpp)

add_state_machine(ClientFsm)
list(FIND srcs ${ClientFsm_C} contains_fsm_c)
if(${contains_fsm_c})
  # do nothing, somehow "NOT ${contains_fsm_c}" does not work ;-(
else(${contains_fsm_c})
  list(APPEND hdrs ${ClientFsm_H})
  list(APPEND srcs ${ClientFsm_C})
endif(${contains_fsm_c})

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)
