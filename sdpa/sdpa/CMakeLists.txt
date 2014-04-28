set (srcs
  capability.cpp
  client.cpp
  com/NetworkStrategy.cpp
  daemon/GenericDaemon.cpp
  daemon/Job.cpp
  daemon/scheduler/CoallocationScheduler.cpp
  daemon/Worker.cpp
  daemon/WorkerManager.cpp
  daemon/agent/Agent.cpp
  daemon/orchestrator/Orchestrator.cpp
)

ADD_LIBRARY(sdpa ${srcs})
target_link_libraries (sdpa
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${FhgCom_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${UTIL_LIBRARIES}
  pnet)
