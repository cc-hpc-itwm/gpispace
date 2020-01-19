extended_add_library (NAME Cache
  NAMESPACE GSPC
  LIBRARIES Boost::base
)
extended_add_library (NAME StencilCache
  NAMESPACE GSPC
  LIBRARIES Util::Generic
            Boost::base
)

#! \todo This is actually tied to src/we/type/stencil_cache.cpp
#! rather than this folder and in fact completely unused here. Move
#! there?
install (FILES
  stencil_cache/callback.hpp
  DESTINATION include/gspc/stencil_cache/
)

extended_add_library (NAME util
  NAMESPACE GSPC
  # SOURCES "gspc/util/Forest.ipp"
  LIBRARIES Boost::serialization
  )
extended_add_library (NAME proto
  NAMESPACE GSPC
  SOURCES "Proto.cpp"
          "GraphTraversalWorkflowEngine.cpp"
          "GreedyScheduler.cpp"
          "MapWorkflowEngine.cpp"
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
          "task/Result.cpp"
          "workflow_engine/ProcessingState.cpp"
          "workflow_engine/State.cpp"
  LIBRARIES GSPC::util
            gspc::logging
            Util::Generic
            RPC
            Boost::base
)

extended_add_executable (NAME Proto_Map
  SOURCES "Proto_Main_Map.cpp"
  LIBRARIES GSPC::proto
)
