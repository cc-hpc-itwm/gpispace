gspc_add_test (NAME drts_add_worker
  SOURCES test_add_worker.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "no"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

gspc_add_test (NAME drts_add_worker_with_ssl_cert
  SOURCES test_add_worker.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "yes"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

gspc_add_test (NAME drts_add_worker_with_descriptions
  SOURCES test_add_worker_with_descriptions.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "no"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

gspc_add_test (NAME drts_add_worker_with_ssl_cert_with_descriptions
  SOURCES test_add_worker_with_descriptions.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "yes"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
