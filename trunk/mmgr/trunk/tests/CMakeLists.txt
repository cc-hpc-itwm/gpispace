# -*- mode: cmake; -*-

## add the CPPUNIT stuff
#include_directories(${CPPUNIT_INCLUDE_DIRS})
#link_directories(${CPPUNIT_LIBRARY_DIRS})
#link_libraries(${CPPUNIT_LIBRARIES} ${CPPUNIT_STATIC_LDFLAGS})

include_directories(${${PROJECT_NAME}_SOURCE_DIR}/mmgr)

#add_subdirectory(fhglog)
add_executable(test_smap test_smap.c)
target_link_libraries(test_smap mmgr)

add_executable(test_heap test_heap.c)
target_link_libraries(test_heap mmgr)

add_executable(test_trie test_trie.c)
target_link_libraries(test_trie mmgr)

add_executable(test_fseg test_fseg.c)
target_link_libraries(test_fseg mmgr)

add_executable(test_ostab test_ostab.c)
target_link_libraries(test_ostab mmgr)

add_executable(test_tmmgr test_tmmgr.c)
target_link_libraries(test_tmmgr mmgr)
