# -*- mode: cmake; -*-
set (pc-srcs pc.cpp)

ADD_LIBRARY(fvm-pc-fake ${pc-srcs})
ADD_LIBRARY(fvm-pc-fake.shared SHARED ${pc-srcs})
set_target_properties(fvm-pc-fake PROPERTIES OUTPUT_NAME fvm-pc)
set_target_properties(fvm-pc-fake.shared PROPERTIES OUTPUT_NAME fvm-pc)

install(TARGETS fvm-pc-fake DESTINATION lib/fake)
install(TARGETS fvm-pc-fake.shared DESTINATION lib/fake)
