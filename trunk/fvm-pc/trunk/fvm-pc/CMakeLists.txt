# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(fvm-pc ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/fvm-pc)
install(TARGETS fvm-pc DESTINATION lib)
