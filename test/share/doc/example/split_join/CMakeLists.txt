set (SPLIT_JOIN_SPLIT_VERSIONS
  join_after_split
  parallel_split_join
)

set (REENTRANT_VERSION not_reentrant)
foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_test (NAME doc_example_${SPLIT_JOIN_MAIN}
    SOURCES test.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/split_join"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
    LIBRARIES gspc
              test-utilities
              fhg-util
              Boost::program_options
              Boost::filesystem
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endforeach()

set (REENTRANT_VERSION reentrant.sequential)
foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_test (NAME doc_example_${SPLIT_JOIN_MAIN}
    SOURCES test.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/split_join"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
         --input 20000
         --expected-output 2666866670000
    LIBRARIES gspc
              test-utilities
              fhg-util
              Boost::program_options
              Boost::filesystem
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endforeach()
