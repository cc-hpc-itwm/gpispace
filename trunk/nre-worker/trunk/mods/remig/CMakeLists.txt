# -*- mode: cmake; -*-

# example module
add_library(init SHARED init.cpp)
add_library(init.up8 SHARED init.up8.cpp)
add_library(readinp SHARED readinp.cpp)
add_library(readvelo SHARED readvelo.cpp)
add_library(calc SHARED calc.cpp)
add_library(update SHARED update.cpp)
add_library(prefsum SHARED prefsum.cpp)
add_library(writeoutp SHARED writeoutp.cpp)
add_library(finalize SHARED finalize.cpp)

install(TARGETS init init.up8 readinp readvelo calc update prefsum writeoutp finalize DESTINATION libexec/remig)
