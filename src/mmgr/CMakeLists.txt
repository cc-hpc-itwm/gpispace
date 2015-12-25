set (sources
  dtmmgr.cpp
  fseg.cpp
  heap.cpp
  ostab.cpp
  smap.cpp
  tmmgr.cpp
  trie.cpp
)

extended_add_library (NAME mmgr SOURCES ${sources})

add_subdirectory (tests)
