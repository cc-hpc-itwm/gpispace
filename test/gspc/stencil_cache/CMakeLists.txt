#! \todo unit tests for stencil cache
add_unit_test (NAME gspc_stencil_cache_2D_standalone
  SOURCES "standalone.cpp"
  ARGS --X 200 --Y 200 --R 2 --M 1000 --C 6 --L 3
  USE_BOOST
  LIBRARIES GSPC::StencilCache
            Util::Generic
            Boost::base
            Boost::program_options
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
)
extended_add_library (NAME stencil_cache_workflow_neighbors_callback
  TYPE SHARED
  SOURCES "workflow/Neighborhood.cpp"
  LIBRARIES Boost::iostreams
            Boost::serialization
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
)
fhg_add_test (NAME gspc_stencil_cache_2D_workflow
  SOURCES "workflow/driver.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  USE_BOOST
  LIBRARIES GSPC::StencilCache
            stencil_cache_workflow_neighbors_callback
            gspc
            test-utilities
            fhg-util
            Util::Generic
            Boost::program_options
            Boost::filesystem
  ARGS --neighbors $<TARGET_FILE:stencil_cache_workflow_neighbors_callback>
       --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/workflow"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --X 30 --Y 30 --R 2
       --plugin-path "${CMAKE_INSTALL_PREFIX}/plugin/$<TARGET_FILE_NAME:Plugin::StencilCache>"
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
