fhg_add_test (handle_generator.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (memory_area.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

fhg_add_test (segment.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
)

extended_add_executable (NAME check_beegfs_segment_requirements
  SOURCES "check_beegfs_segment_requirements.cpp"
  LIBRARIES GPISpace::vmem
            Util::Generic
            test-utilities
            Boost::program_options
)

file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  CONTENT "execute_process (COMMAND \"$<TARGET_FILE:check_beegfs_segment_requirements.exe>\"
                                    \"${SHARED_DIRECTORY_FOR_TESTS}\"
             WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
             RESULT_VARIABLE check_beegfs_segment_requirements_result
           )

           if (NOT \${check_beegfs_segment_requirements_result} EQUAL 0)
             list (APPEND CTEST_CUSTOM_TESTS_IGNORE gpi-space_test_beegfs_area)
           endif()"
)

fhg_add_test (beegfs_area.cpp
  PROJECT gpi-space_test
  BOOST_UNIT_TEST
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  INCLUDE_DIRECTORIES SYSTEM PRIVATE "${LIBRT_INCLUDE_DIR}"
  LINK_LIBRARIES gpi-space-pc-segment
                 gpi-space-pc-memory
                 fhgcom
                 fhglog
                 Boost::filesystem
                 ${CMAKE_THREAD_LIBS_INIT}
                 fhg-util
                 test-utilities
)
