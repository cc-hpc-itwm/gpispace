# tiberiu.rotaru@itwm.fraunhofer.de

add_subdirectory (orchestrator)

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
  )

add_library (sdpa-daemon ${srcs})

target_link_libraries (sdpa-daemon
  ${UTIL_LIBRARIES} ${ZLIB_LIBRARIES} sdpa-com sdpa-events sdpa-daemon)
