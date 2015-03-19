ADD_LIBRARY(sdpa
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

target_link_libraries (sdpa
  gpi-space-pc-client-static
  gpi-space-pc-segment
  Network
  RPC
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  fhgcom
  ${Boost_THREAD_LIBRARY}
  fhg-util
  pnet)
