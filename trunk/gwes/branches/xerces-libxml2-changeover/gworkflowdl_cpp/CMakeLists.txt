
file(GLOB gwdl-hdrs "include/gwdl/*.h" "include/gwdl/*.hpp")
file(GLOB gwdl-srcs "src/*.cpp")

# library gworkflowdl_cpp
ADD_LIBRARY(gworkflowdl_cpp ${gwdl-srcs})
target_link_libraries(gworkflowdl_cpp ${UUID_LIBRARY})
target_link_libraries(gworkflowdl_cpp ${FHGLOG_LIBRARY})
target_link_libraries(gworkflowdl_cpp ${XERCESC_LIBRARIES})
target_link_libraries(gworkflowdl_cpp ${LIBXML2_LIBRARIES})
target_link_libraries(gworkflowdl_cpp ${LIBXSLT_LIBRARIES})
target_link_libraries(gworkflowdl_cpp ${Boost_LIBRARIES})

install(FILES ${gwdl-hdrs} DESTINATION include/gwdl)
install(TARGETS gworkflowdl_cpp ARCHIVE DESTINATION lib)
