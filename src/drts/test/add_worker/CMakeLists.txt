fhg_add_test (NAME drts_add_worker
  SOURCES test_add_worker.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:RPC>"
       --ssl-cert "no"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

fhg_add_test (NAME drts_add_worker_with_ssl_cert
  SOURCES test_add_worker.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:RPC>"
       --ssl-cert "yes"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

fhg_add_test (NAME drts_add_worker_with_descriptions
  SOURCES test_add_worker_with_descriptions.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:RPC>"
       --ssl-cert "no"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

fhg_add_test (NAME drts_add_worker_with_ssl_cert_with_descriptions
  SOURCES test_add_worker_with_descriptions.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:RPC>"
       --ssl-cert "yes"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
