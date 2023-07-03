if (BUILD_TESTING)
  gspc_add_library (NAME transform_file_type
    TYPE STATIC
    SOURCES "type.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    LIBRARIES we-dev
              Boost::filesystem
  )

  gspc_add_library (NAME transform_file_map-impl
    TYPE MODULE
    SOURCES "implementation.cpp"
    LIBRARIES GPISpace::header-only
              transform_file_type
    INCLUDE_DIRECTORIES
      PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../.."
        "${PROJECT_SOURCE_DIR}/share/doc/example"
  )

  math (EXPR size_input "20 << 20")
  math (EXPR size_output "20 << 20")
  math (EXPR size_block "2 << 20")
  math (EXPR num_block "120")
endif()

fhg_add_test (NAME doc_example_map_transform_file
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/map"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:transform_file_map-impl>"
       --size-input ${size_input}
       --size-output ${size_output}
       --size-block ${size_block}
       --num-block ${num_block}
  LIBRARIES gspc
            test-utilities
            transform_file_type
            fhg-util
            Boost::program_options
            Boost::filesystem
  DEPENDS transform_file_map-impl
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
