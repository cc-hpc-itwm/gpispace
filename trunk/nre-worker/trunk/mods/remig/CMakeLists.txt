# -*- mode: cmake; -*-

# example module
add_library(init SHARED init.cpp)

install(TARGETS init DESTINATION libexec/remig)
