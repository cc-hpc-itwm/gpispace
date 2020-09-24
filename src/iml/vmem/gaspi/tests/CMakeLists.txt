fhg_add_test (NAME iml-vmem_handle_generator
  SOURCES handle_generator.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES IML::vmem-pc-segment
            IML::vmem-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            IML::fhg-util
)

fhg_add_test (NAME iml-vmem_memory_area
  SOURCES memory_area.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES IML::vmem-pc-segment
            IML::vmem-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            IML::fhg-util
)

fhg_add_test (NAME iml-vmem_segment
  SOURCES segment.cpp
  USE_BOOST
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES IML::vmem-pc-segment
            IML::vmem-pc-memory
            fhgcom
            ${CMAKE_THREAD_LIBS_INIT}
            IML::fhg-util
)

extended_add_executable (NAME iml-check_beegfs_segment_requirements
  SOURCES "check_beegfs_segment_requirements.cpp"
  LIBRARIES IML::vmem-client
            Util::Generic
            IML::test-utilities
            Boost::program_options
)

file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  CONTENT "execute_process (COMMAND \"$<TARGET_FILE:iml-check_beegfs_segment_requirements.exe>\"
                                    \"${SHARED_DIRECTORY_FOR_TESTS}\"
             WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
             RESULT_VARIABLE iml_check_beegfs_segment_requirements_result
           )

           if (NOT \${iml_check_beegfs_segment_requirements_result} EQUAL 0)
             list (APPEND CTEST_CUSTOM_TESTS_IGNORE iml-vmem_beegfs_area)
           endif()"
)

fhg_add_test (NAME iml-vmem_beegfs_area
  SOURCES beegfs_area.cpp
  USE_BOOST
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
  LIBRARIES IML::vmem-pc-segment
            IML::vmem-pc-memory
            fhgcom
            Boost::filesystem
            ${CMAKE_THREAD_LIBS_INIT}
            IML::fhg-util
            IML::test-utilities
)

fhg_add_test (NAME iml-api
  SOURCES "api.cpp"
  USE_BOOST
  LIBRARIES IML::vmem-pc-client-static
            Util::Generic
)

fhg_add_test (NAME iml-vmem_api_types_match_real_types
  SOURCES "api_types_match_real_types.cpp"
  LIBRARIES GASPI::static
            Util::Generic
  USE_BOOST
)
