set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror")

set (CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb")
set (CMAKE_C_FLAGS_PROFILE "-O0 -g -pg")

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (sources
  mmgr/dtmmgr.c
  mmgr/fseg.c
  mmgr/heap.c
  mmgr/ostab.c
  mmgr/smap.c
  mmgr/tmmgr.c
  mmgr/trie.c
)

add_library (mmgr ${sources})
add_library (mmgr.shared SHARED ${sources})
set_target_properties (mmgr.shared PROPERTIES OUTPUT_NAME mmgr)

install (TARGETS mmgr
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries
)
install (TARGETS mmgr.shared
  LIBRARY
  DESTINATION lib
  COMPONENT libraries
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

set (apps
  iheapsort
  trie_vs_smap
  perftmmgr
)

foreach (app ${apps})
  add_executable (${app} app/${app}.c)
  target_link_libraries (${app} mmgr)
endforeach()
