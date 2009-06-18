
file(GLOB jobFSM-src *.cpp *.hpp)

ADD_LIBRARY(jobFSM ${jobFSM-src})
ADD_executable(jobFSM-exe ${jobFSM-src})

install(FILES ${hdrs} DESTINATION include/sdpa/jobFSM)
install(TARGETS jobFSM ARCHIVE DESTINATION lib)
