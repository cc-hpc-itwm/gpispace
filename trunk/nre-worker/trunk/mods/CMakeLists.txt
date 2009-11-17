# -*- mode: cmake; -*-

file(GLOB mod_srcs "*-mod.cpp")
list(SORT mod_srcs)
set(mods)
foreach (mod_src ${mod_srcs})
  # get the filename without extension
  string(REGEX REPLACE "(.*/)?(.*)\\.cpp.*" "\\2" mod_name ${mod_src})
  message(STATUS "adding module ${mod_name} (using ${mod_src})")

  add_library(${mod_name} SHARED ${mod_src})
  install(TARGETS ${mod_name} DESTINATION libexec)
endforeach(mod_src ${mod_srcs})

add_subdirectory(remig)
