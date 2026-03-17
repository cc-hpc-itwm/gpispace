gspc_add_test (NAME drts_workflow_response_using_secure_communication
  SOURCES test_using_secure_communication.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "no"
       --xpnet "workflow_response"
       --topology "worker:2"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

gspc_add_test (NAME drts_workflow_response_one_response_waits
  SOURCES test_one_response_waits.cpp
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
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

gspc_add_test (NAME drts_workflow_response_workflow_response_fails
  SOURCES test_workflow_response_fails.cpp
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
            fmt
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

gspc_add_test (NAME drts_workflow_response_with_ssl_cert_using_secure_communication
  SOURCES test_using_secure_communication.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:gspc_rpc>"
       --ssl-cert "yes"
       --xpnet "workflow_response_expression"
       --topology "worker:1"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

gspc_add_test (NAME drts_workflow_response_with_ssl_cert_one_response_waits
  SOURCES test_one_response_waits.cpp
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
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

gspc_add_test (NAME drts_workflow_response_with_ssl_cert_workflow_response_fails
  SOURCES test_workflow_response_fails.cpp
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
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
