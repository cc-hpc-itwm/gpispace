set (sources
  dtmmgr.cpp
  tmmgr.cpp
)

extended_add_library (NAME mmgr SOURCES ${sources})

add_subdirectory (tests)
