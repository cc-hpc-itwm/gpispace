# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp" "*.h")

install(FILES ${hdrs} DESTINATION include/fvm)
