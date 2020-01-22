extended_add_library (NAME proto-example
  NAMESPACE GSPC
  SOURCES "workflow_engine/GraphTraversalWorkflowEngine.cpp"
          "workflow_engine/MapWorkflowEngine.cpp"
          "workflow_engine/SequenceWorkflowEngine.cpp"
  LIBRARIES GSPC::proto
)

extended_add_library (NAME map
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/map.cpp"
  LIBRARIES GSPC::proto GSPC::proto-example
)
extended_add_library (NAME sequence
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/sequence.cpp"
  LIBRARIES GSPC::proto GSPC::proto-example
)
extended_add_library (NAME graph
  TYPE MODULE VISIBILITY_HIDDEN
  NAMESPACE ExampleModule
  SOURCES "modules/graph.cpp"
  LIBRARIES GSPC::proto GSPC::proto-example
)

extended_add_executable (NAME Proto_Map
  SOURCES "Proto_Main_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-map>"
  LIBRARIES GSPC::proto GSPC::proto-example
)
extended_add_executable (NAME Proto_Sequence
  SOURCES "Proto_Main_Sequence.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-sequence>"
  LIBRARIES GSPC::proto GSPC::proto-example
)
extended_add_executable (NAME Proto_Graph_Static_Map
  SOURCES "Proto_Main_Graph_Static_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto GSPC::proto-example
)

extended_add_executable (NAME Proto_Graph_Dynamic_Map
  SOURCES "Proto_Main_Graph_Dynamic_Map.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto GSPC::proto-example
)

extended_add_executable (NAME Proto_Graph_Nary_Tree
  SOURCES "Proto_Main_Graph_Nary_Tree.cpp"
  COMPILE_DEFINITIONS MODULE="$<TARGET_FILE:ExampleModule-graph>"
  LIBRARIES GSPC::proto GSPC::proto-example
)
