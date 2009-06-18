
file(GLOB orchFSM-src *.cpp *.hpp)

ADD_LIBRARY(orchFSM ${orchFSM-src})
ADD_executable(orchFSM-exe ${orchFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/orchFSM)
install(TARGETS orchFSM ARCHIVE DESTINATION lib)
