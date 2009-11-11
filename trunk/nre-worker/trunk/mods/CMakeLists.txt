# -*- mode: cmake; -*-

# example module
add_library(example-mod SHARED example-mod.cpp)
add_library(pc-test SHARED pc-test.cpp)

add_library(mod-a SHARED mod-a.cpp)
add_library(mod-b SHARED mod-b.cpp)

install(TARGETS example-mod pc-test mod-a mod-b DESTINATION libexec)
