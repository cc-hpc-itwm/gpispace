# tiberiu.rotaru@itwm.fraunhofer.de

set (srcs
  GenericDaemon.cpp
  Job.cpp
  JobManager.cpp
  scheduler/SchedulerBase.cpp
  scheduler/CoallocationScheduler.cpp
  scheduler/SimpleScheduler.cpp
  Worker.cpp
  WorkerManager.cpp
  agent/Agent.cpp
  orchestrator/Orchestrator.cpp
  )

add_library (sdpa-daemon ${srcs})

target_link_libraries (sdpa-daemon
  ${UTIL_LIBRARIES} ${ZLIB_LIBRARIES} pnet sdpa ${Boost_THREAD_LIBRARY})
