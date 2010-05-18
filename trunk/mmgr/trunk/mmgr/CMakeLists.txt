file(GLOB hdrs *.h)
file(GLOB srcs "*.c")

ADD_LIBRARY(mmgr ${hdrs} ${srcs})
ADD_LIBRARY(mmgr.shared SHARED ${hdrs} ${srcs})
set_target_properties(mmgr.shared PROPERTIES OUTPUT_NAME mmgr)

install(FILES ${hdrs} DESTINATION include/mmgr)
install(TARGETS mmgr DESTINATION lib)
install(TARGETS mmgr.shared DESTINATION lib)
