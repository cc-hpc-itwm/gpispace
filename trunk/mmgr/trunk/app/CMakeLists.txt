# -*- mode: cmake; -*-

## add the CPPUNIT stuff
#include_directories(${CPPUNIT_INCLUDE_DIRS})
#link_directories(${CPPUNIT_LIBRARY_DIRS})
#link_libraries(${CPPUNIT_LIBRARIES} ${CPPUNIT_STATIC_LDFLAGS})

include_directories(${${PROJECT_NAME}_SOURCE_DIR}/mmgr)

#add_subdirectory(fhglog)
add_executable(iheapsort iheapsort.c)
target_link_libraries(iheapsort mmgr)

add_executable(trie_vs_smap trie_vs_smap.c)
target_link_libraries(trie_vs_smap mmgr)

add_executable(perftmmgr perftmmgr.c)
target_link_libraries(perftmmgr mmgr)
