file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

add_subdirectory(orchestrator)
add_subdirectory(aggregator)
add_subdirectory(nre)

add_subdirectory(jobFSM)
add_subdirectory(daemonFSM)

ADD_LIBRARY(sdpa-daemon ${hdrs} ${srcs})
target_link_libraries(sdpa-daemon sdpa-jobFSM_SMC sdpa-daemonFSM_SMC)
target_link_libraries(sdpa-daemon ${GWES_LIBRARY})
target_link_libraries(sdpa-daemon ${GWDL_LIBRARY})
target_link_libraries(sdpa-daemon ${XERCESC_LIBRARIES})
target_link_libraries(sdpa-daemon ${LIBXML2_LIBRARIES})
target_link_libraries(sdpa-daemon ${LIBXSLT_LIBRARIES})

install(FILES ${hdrs} DESTINATION include/sdpa/daemon)
install(TARGETS sdpa-daemon ARCHIVE DESTINATION lib)
