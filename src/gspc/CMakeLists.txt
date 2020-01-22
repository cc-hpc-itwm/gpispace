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
          "SequenceWorkflowEngine.cpp"
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

extended_add_library (NAME map
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/map.cpp"
  LIBRARIES GSPC::proto
)
extended_add_library (NAME sequence
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/sequence.cpp"
  LIBRARIES GSPC::proto
)
extended_add_library (NAME graph
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/graph.cpp"
  LIBRARIES GSPC::proto
)

extended_add_executable (NAME Proto_Map
  SOURCES "Proto_Main_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-map>"
  LIBRARIES GSPC::proto
)
extended_add_executable (NAME Proto_Sequence
  SOURCES "Proto_Main_Sequence.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-sequence>"
  LIBRARIES GSPC::proto
)
extended_add_executable (NAME Proto_Graph_Static_Map
  SOURCES "Proto_Main_Graph_Static_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto
)

extended_add_executable (NAME Proto_Graph_Dynamic_Map
  SOURCES "Proto_Main_Graph_Dynamic_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto
)

extended_add_executable (NAME Proto_Graph_Nary_Tree
  SOURCES "Proto_Main_Graph_Nary_Tree.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto
)
