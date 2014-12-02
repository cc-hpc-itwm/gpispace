set (GSPC_NODEFILE_FOR_TESTS "$ENV{GSPC_NODEFILE_FOR_TESTS}")
if (NOT GSPC_NODEFILE_FOR_TESTS)
  message (FATAL_ERROR "Cannot run tests, GSPC_NODEFILE_FOR_TESTS environment variable is not set")
endif()
