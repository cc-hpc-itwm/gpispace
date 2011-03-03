add_executable(nre main.cpp)
#target_link_libraries(nre sdpa-nre)
target_link_libraries(nre sdpa-events)
target_link_libraries(nre sdpa-daemon)
target_link_libraries(nre sdpa-util)
target_link_libraries(nre sdpa-com)
target_link_libraries(nre sdpa)
target_link_libraries(nre seda)
#target_link_libraries(nre seda-comm)
target_link_libraries(nre ${Boost_LIBRARIES})
target_link_libraries(nre ${WE_PRECOMPILE_LIBRARIES})

install(TARGETS nre
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
