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
add_library (mmgr.shared SHARED ${sources})
set_target_properties (mmgr.shared
  PROPERTIES OUTPUT_NAME mmgr
             VERSION     1
)

install (TARGETS mmgr.shared
  LIBRARY
  DESTINATION lib
  COMPONENT runtime
)

add_subdirectory (tests)
