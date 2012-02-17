set (hdrs
  bool.h
  dtmmgr.h
  error.h
  fseg.h
  heap.h
  null.h
  ostab.h
  smap.h
  tmmgr.h
  trie.h
  unused.h
  word.h
  )

set (srcs
  dtmmgr.c
  fseg.c
  heap.c
  ostab.c
  smap.c
  tmmgr.c
  trie.c
  )

ADD_LIBRARY(mmgr ${hdrs} ${srcs})
ADD_LIBRARY(mmgr.shared SHARED ${hdrs} ${srcs})
set_target_properties(mmgr.shared PROPERTIES OUTPUT_NAME mmgr)

install(FILES ${hdrs}
  DESTINATION include/mmgr
  COMPONENT headers)
install(TARGETS mmgr
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
install(TARGETS mmgr.shared
  LIBRARY
  DESTINATION lib
  COMPONENT libraries)
