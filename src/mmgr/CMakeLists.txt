set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")

set (sources
  dtmmgr.c
  fseg.c
  heap.c
  ostab.c
  smap.c
  tmmgr.c
  trie.c
)

add_library (mmgr ${sources})

add_subdirectory (tests)
