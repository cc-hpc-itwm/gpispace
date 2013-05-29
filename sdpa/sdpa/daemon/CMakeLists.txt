# tiberiu.rotaru@itwm.fraunhofer.de

set (srcs
  DaemonFSM.cpp
  GenericDaemon.cpp
  Job.cpp
  JobEventHandlers.cpp
  JobImpl.cpp
  JobManager.cpp
  SchedulerImpl.cpp
  SynchronizedQueue.cpp
  Worker.cpp
  WorkerManager.cpp

  agent/Agent.cpp
  orchestrator/Orchestrator.cpp
  )

add_library (sdpa-daemon ${srcs})

target_link_libraries (sdpa-daemon
  ${UTIL_LIBRARIES} ${ZLIB_LIBRARIES} pnet sdpa-com sdpa-events sdpa-daemon)
