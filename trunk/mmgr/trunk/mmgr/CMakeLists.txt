file(GLOB hdrs *.h)
file(GLOB srcs "*.c")

include_directories(${${PROJECT_NAME}_SOURCE_DIR}/mmgr)

ADD_LIBRARY(mmgr ${hdrs} ${srcs})
#TARGET_LINK_LIBRARIES(mmgr -pg)

install(FILES ${hdrs} DESTINATION include/mmgr)
install(TARGETS mmgr ARCHIVE DESTINATION lib)
