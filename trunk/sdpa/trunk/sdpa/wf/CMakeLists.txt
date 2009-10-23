#
file(GLOB srcs "*.cpp")
file(GLOB hdrs "*.hpp")

install(FILES ${hdrs} DESTINATION include/sdpa/wf)
