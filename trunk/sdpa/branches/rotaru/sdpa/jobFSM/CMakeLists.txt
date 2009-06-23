file(GLOB jobFSM-src *.cpp *.hpp)

ADD_LIBRARY(jobFSM ${jobFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM)
install(TARGETS jobFSM ARCHIVE DESTINATION lib)
