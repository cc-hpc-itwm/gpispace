set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")

include_directories (${CMAKE_SOURCEX_DIR}/mmgr)

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

set (tests
  heap
  dtmmgr
  fseg
  ostab
  smap
  tmmgr
  trie
)

foreach (test ${tests})
  fhg_add_test (tests/${test}.c PROJECT test_mmgr LINK_LIBRARIES mmgr)
endforeach()
