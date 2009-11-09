file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

install(FILES ${hdrs} DESTINATION include/sdpa/daemon/nre)
