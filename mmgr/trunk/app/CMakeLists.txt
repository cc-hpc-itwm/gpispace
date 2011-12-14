# -*- mode: cmake; -*-

#add_subdirectory(fhglog)
add_executable(iheapsort iheapsort.c)
target_link_libraries(iheapsort mmgr)

add_executable(trie_vs_smap trie_vs_smap.c)
target_link_libraries(trie_vs_smap mmgr)

add_executable(perftmmgr perftmmgr.c)
target_link_libraries(perftmmgr mmgr)
