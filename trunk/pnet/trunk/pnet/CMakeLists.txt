# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(pnet ${srcs})
ADD_LIBRARY(pnet.shared SHARED ${srcs})
set_target_properties(pnet.shared PROPERTIES OUTPUT_NAME pnet)

install(FILES ${hdrs} DESTINATION include/pnet)
install(TARGETS pnet DESTINATION lib)
install(TARGETS pnet.shared DESTINATION lib)
