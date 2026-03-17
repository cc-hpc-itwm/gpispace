gspc_add_library (NAME share_example_parallel_reduce_module_call_Task
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "Task.cpp"
  LIBRARIES
    PRIVATE gspc_util
    PRIVATE Boost::serialization
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)

gspc_add_library (NAME share_example_parallel_reduce_module_call_JobExecutor
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "JobExecutor.cpp"
  LIBRARIES
    PRIVATE gspc_rpc
    PRIVATE gspc_util
    PRIVATE GSPC::share_example_parallel_reduce_module_call_Task
    PRIVATE Boost::base
    PRIVATE fmt
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)
gspc_add_library (NAME share_example_parallel_reduce_module_call_JobServer
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "JobServer.cpp"
  LIBRARIES
    PRIVATE gspc_rpc
    PRIVATE gspc_util
    PRIVATE GSPC::share_example_parallel_reduce_module_call_Task
    PRIVATE Boost::base
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)

gspc_add_test (NAME share_example_parallel_reduce_module_call
  SOURCES "driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}"
  LIBRARIES gspc
            gspc_testing
            Boost::program_options
            GSPC::share_example_parallel_reduce_module_call_JobServer
            fmt
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  COMPILE_DEFINITIONS
    PRIVATE RPC_LIB="$<TARGET_FILE:gspc_rpc>"
    PRIVATE JOB_EXECUTOR_LIB="$<TARGET_FILE:GSPC-share_example_parallel_reduce_module_call_JobExecutor>"
    PRIVATE TASK_LIB="$<TARGET_FILE:GSPC-share_example_parallel_reduce_module_call_Task>"
)
