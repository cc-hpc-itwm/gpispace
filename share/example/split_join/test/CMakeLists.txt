# mirko.rahn@itwm.fraunhofer.de

set (REENTRANT_VERSION not_reentrant)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_test (test.cpp
    PROJECT share_example_${SPLIT_JOIN_MAIN}
    BOOST_UNIT_TEST
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_BINARY_DIR}/../"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
    LINK_LIBRARIES gspc
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

  fhg_add_test (test.cpp
    PROJECT share_example_${SPLIT_JOIN_MAIN}
    BOOST_UNIT_TEST
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_BINARY_DIR}/../"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --main "${SPLIT_JOIN_MAIN}"
         --input 10000
         --expected-output 333383335000
         --input 20000
         --expected-output 2666866670000
    LINK_LIBRARIES gspc
                   test-utilities
                   fhg-util
                   Boost::program_options
                   Boost::filesystem
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endforeach()
