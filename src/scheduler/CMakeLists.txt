gspc_add_library (NAME gspc_scheduler
  TYPE STATIC
  SOURCES capability.cpp
          client.cpp
          id_generator.cpp
          com/NetworkStrategy.cpp
          daemon/Agent.cpp
          daemon/GetSchedulerType.cpp
          daemon/Job.cpp
          daemon/NotificationEvent.cpp
          daemon/Worker.cpp
          daemon/WorkerManager.cpp
          daemon/scheduler/CostAwareWithWorkStealingStrategy.cpp
          daemon/scheduler/CoallocationScheduler.cpp
          daemon/scheduler/GreedyScheduler.cpp
          daemon/scheduler/SingleAllocationScheduler.cpp
          daemon/scheduler/Reservation.cpp
          events/Codec.cpp
          events/EventHandler.cpp
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES PRIVATE GPISpace::header-only
            PUBLIC gspc_we
            PUBLIC fhgcom
            PUBLIC gspc_util
            PUBLIC gspc_logging
            PUBLIC gspc_scheduler_types
            PUBLIC $<$<BOOL:${GSPC_WITH_IML}>:gspc_iml_client>
            PRIVATE scheduler-types
            PUBLIC Boost::thread
            PUBLIC Boost::serialization
            PRIVATE Boost::date_time
)
