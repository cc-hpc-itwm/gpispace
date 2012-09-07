add_subdirectory(plugin)

set (hdrs
  MasterMapReduceWorkflowEngine.hpp
  WorkerMapReduceWorkflowEngine.hpp
  Mapper.hpp
  Reducer.hpp
)

install(FILES ${hdrs}
  DESTINATION include/sdpa/engine
  COMPONENT headers)
