#
file(GLOB srcs "*.cpp")
file(GLOB hdrs "*.hpp")

ADD_LIBRARY(sdpa-wf ${srcs} ${hdrs})
