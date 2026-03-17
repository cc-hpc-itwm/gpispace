math (EXPR num_bytes "192 << 20")

gspc_add_test (NAME doc_example_vmem_1_to_n
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/vmem_1_to_n"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --num-bytes ${num_bytes}
  LIBRARIES gspc
            gspc_testing
            transform_file_type
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
