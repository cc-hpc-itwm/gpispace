add_unit_test (NAME started_process_promise
  SOURCES "started_process_promise.cpp"
  DESCRIPTION "pipe used for communicating with a rif-started process during startup"
  USE_BOOST
  LIBRARIES gspc_rif_started_process_promise
            gspc_util
            gspc_testing
            PRIVATE fmt
)
