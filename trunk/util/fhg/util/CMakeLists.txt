# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
install(FILES ${hdrs} DESTINATION include/fhg/util)
