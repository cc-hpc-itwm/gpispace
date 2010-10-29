add_executable(orchestrator main.cpp)
#add_dependencies(orchestrator sdpa-orchestrator)
#target_link_libraries(orchestrator sdpa-orchestrator)
target_link_libraries(orchestrator sdpa-events)
target_link_libraries(orchestrator sdpa-daemon)
target_link_libraries(orchestrator sdpa-util)
target_link_libraries(orchestrator sdpa-com)
target_link_libraries(orchestrator sdpa)
target_link_libraries(orchestrator seda)
target_link_libraries(orchestrator seda-comm)
target_link_libraries(orchestrator ${Boost_LIBRARIES})

install(TARGETS orchestrator
  RUNTIME
  DESTINATION bin
  COMPONENT applications)
