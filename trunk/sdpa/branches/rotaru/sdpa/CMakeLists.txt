add_subdirectory(util)
add_subdirectory(events)

#obsolete
#add_subdirectory(wf)
add_subdirectory(modules)
add_subdirectory(daemon)
add_subdirectory(client)

file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(sdpa ${hdrs} ${srcs})
target_link_libraries(sdpa seda)      # ${SEDA_LIBRARY}
target_link_libraries(sdpa seda-comm) # ${SEDA_COMM_LIBRARY}
target_link_libraries(sdpa ${FHGLOG_LIBRARY})

install(FILES ${hdrs} DESTINATION include/sdpa)
install(TARGETS sdpa ARCHIVE DESTINATION lib)
