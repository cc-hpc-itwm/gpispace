file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_state_machine(ClientFsm)
set(hdrs ${hdrs} ${ClientFsm_H})

ADD_LIBRARY(sdpa-client ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/sdpa/client)
install(TARGETS sdpa-client ARCHIVE DESTINATION lib)
