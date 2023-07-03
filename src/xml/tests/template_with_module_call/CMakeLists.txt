fhg_add_test (NAME xml_template_with_module_call_cast_From_To
  SOURCES "cast_From_To.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
            Util::Generic
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (NAME xml_template_with_module_call_print_T
  SOURCES "print_T.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
            Util::Generic
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
