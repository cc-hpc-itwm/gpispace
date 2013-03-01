set (hdrs
  exceptions.hpp
  GenericDaemonActions.hpp
  GenericDaemon.hpp
  IComm.hpp
  #IWorkflowEngine.hpp
  #JobFSMActions.hpp
  Job.hpp
  JobImpl.hpp
  JobManager.hpp
  NotificationEvent.hpp
  NotificationService.hpp
  Observable.hpp
  Observer.hpp
  Scheduler.hpp
  SchedulerImpl.hpp
  SynchronizedQueue.hpp
  Worker.hpp
  WorkerManager.hpp
  )

set (srcs
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

add_subdirectory(orchestrator)
add_subdirectory(agent)

add_subdirectory(jobFSM)
add_subdirectory(daemonFSM)

ADD_LIBRARY(sdpa-daemon ${hdrs} ${srcs})

if (USE_BOOST_SC)
	target_link_libraries(sdpa-daemon sdpa-jobFSM_BSC sdpa-daemonFSM_BSC)
endif (USE_BOOST_SC)

if (USE_BOOST_MSM)
	target_link_libraries(sdpa-daemon sdpa-daemonFSM_BMSM)
endif (USE_BOOST_MSM)

target_link_libraries(sdpa-daemon ${UTIL_LIBRARIES})
target_link_libraries(sdpa-daemon ${ZLIB_LIBRARIES})
target_link_libraries(sdpa-daemon sdpa-com)

install(FILES ${hdrs}
  DESTINATION include/sdpa/daemon
  COMPONENT headers)
install(TARGETS sdpa-daemon
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries)
