# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
set (pc-srcs pc.cpp)

ADD_LIBRARY(fvm-pc ${hdrs} ${pc-srcs})
ADD_LIBRARY(fvm-pc.shared SHARED ${hdrs} ${pc-srcs})
set_target_properties(fvm-pc.shared PROPERTIES OUTPUT_NAME fvm-pc)

install(FILES ${hdrs} DESTINATION include/fvm-pc)
install(TARGETS fvm-pc DESTINATION lib)
install(TARGETS fvm-pc.shared DESTINATION lib)

if (WITH_FAKE_PC)
  add_subdirectory(fake)
endif (WITH_FAKE_PC)
