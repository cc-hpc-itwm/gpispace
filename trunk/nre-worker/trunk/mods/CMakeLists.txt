# -*- mode: cmake; -*-

# example module
add_library(example-mod SHARED example-mod.cpp)
add_library(pc-test SHARED pc-test.cpp)
#target_link_libraries(pc-test ${FVM_PC_LIBRARY})

add_library(mod-a SHARED mod-a.cpp)
add_library(mod-b SHARED mod-b.cpp)
