# -*- mode: cmake; -*-

# example module
add_library(example-mod SHARED example-mod.cpp)
add_library(pc-test SHARED pc-test.cpp)
add_library(mod-a SHARED mod-a.cpp)
add_library(mod-b SHARED mod-b.cpp)
add_library(log-test SHARED log-test.cpp)
add_library(dummy SHARED dummy.cpp)


# fvm
add_library(fvm-mod  SHARED fvm-mod.cpp)

install(TARGETS example-mod pc-test mod-a mod-b log-test fvm-mod DESTINATION libexec)

add_subdirectory(remig)

