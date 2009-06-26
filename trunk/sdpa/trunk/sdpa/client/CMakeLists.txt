file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

set(hdrs ${hdrs} "ClientFsm_sm.h")
set(srcs ${srcs} "ClientFsm_sm.cpp")

add_state_machine(ClientFsm)

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)
