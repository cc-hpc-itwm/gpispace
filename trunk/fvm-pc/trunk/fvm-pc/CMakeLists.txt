# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
set (pc-srcs pc.cpp)
set (pc-fake-srcs pc_fake.cpp)

ADD_LIBRARY(fvm-pc ${hdrs} ${pc-srcs})
ADD_LIBRARY(fvm-pc.shared SHARED ${hdrs} ${pc-srcs})
set_target_properties(fvm-pc.shared PROPERTIES OUTPUT_NAME fvm-pc)

ADD_LIBRARY(fvm-pc_fake ${hdrs} ${pc-fake-srcs})
ADD_LIBRARY(fvm-pc_fake.shared SHARED ${hdrs} ${pc-fake-srcs})
set_target_properties(fvm-pc_fake.shared PROPERTIES OUTPUT_NAME fvm-pc_fake)
target_link_libraries(fvm-pc_fake ${MMGR_LIBRARY})

install(FILES ${hdrs} DESTINATION include/fvm-pc)
install(TARGETS fvm-pc DESTINATION lib)
install(TARGETS fvm-pc.shared DESTINATION lib)

install(TARGETS fvm-pc_fake DESTINATION lib)
install(TARGETS fvm-pc_fake.shared DESTINATION lib)
