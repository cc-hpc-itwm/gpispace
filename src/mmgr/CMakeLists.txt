set (sources
  dtmmgr.cpp
  heap.cpp
  ostab.cpp
  tmmgr.cpp
  trie.cpp
)

extended_add_library (NAME mmgr SOURCES ${sources})

add_subdirectory (tests)
