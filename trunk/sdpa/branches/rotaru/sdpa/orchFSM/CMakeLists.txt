
file(GLOB orchFSM-src *.cpp *.hpp)

ADD_LIBRARY(orchFSM ${orchFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/orchFSM)
install(TARGETS orchFSM ARCHIVE DESTINATION lib)
