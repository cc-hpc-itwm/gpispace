extended_add_library (NAME util
  NAMESPACE GSPC
  # SOURCES "gspc/util/Forest.ipp"
  LIBRARIES Boost::serialization
  )
extended_add_library (NAME proto
  NAMESPACE GSPC
  SOURCES "Proto.cpp"
          "GreedyScheduler.cpp"
          "RemoteInterface.cpp"
          "Resource.cpp"
          "ScopedRuntimeSystem.cpp"
          "Task.cpp"
          "Worker.cpp"
          "comm/runtime_system/remote_interface/Client.cpp"
          "comm/scheduler/worker/Client.cpp"
          "comm/scheduler/worker/Server.cpp"
          "comm/worker/scheduler/Client.cpp"
          "comm/worker/scheduler/Server.cpp"
          "job/ID.cpp"
          "remote_interface/ConnectionAndPID.cpp"
          "remote_interface/ID.cpp"
          "remote_interface/strategy/Thread.cpp"
          "resource/ID.cpp"
          "resource_manager/Coallocation.cpp"
          "resource_manager/Trivial.cpp"
          "resource_manager/WithPreferences.cpp"
          "rpc/TODO.cpp"
          "task/ID.cpp"
          "task/Implementation.cpp"
          "task/Result.cpp"
          "workflow_engine/ProcessingState.cpp"
  LIBRARIES GSPC::util
            gspc::logging
            Util::Generic
            RPC
            Boost::base
)

add_subdirectory (example)

extended_add_executable (NAME DumpState
  SOURCES "Dump.cpp"
  LIBRARIES GSPC::proto Util::Generic
)
