gspc_add_test (NAME gspc_testing-measure_average_time
  SOURCES "measure_average_time.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
  PERFORMANCE_TEST
)

gspc_add_test (NAME gspc_testing-printer-chrono
  SOURCES "printer/chrono.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-future
  SOURCES "printer/future.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-generic
  SOURCES "printer/generic.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-hard_integral_typedef
  SOURCES "printer/hard_integral_typedef.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-list
  SOURCES "printer/list.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-map
  SOURCES "printer/map.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-multimap
  SOURCES "printer/multimap.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-multiset
  SOURCES "printer/multiset.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-optional
  SOURCES "printer/optional.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-pair
  SOURCES "printer/pair.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-set
  SOURCES "printer/set.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-tuple
  SOURCES "printer/tuple.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-type_info
  SOURCES "printer/type_info.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-printer-vector
  SOURCES "printer/vector.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-random
  SOURCES "random.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-random_string
  SOURCES "random_string.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-require_compiletime
  SOURCES "require_compiletime.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-require_container_is_permutation
  SOURCES "require_container_is_permutation.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

# Tests that are expected to fail
gspc_add_test (NAME gspc_testing-require_container_is_permutation_negative-different_size_is_noticed
  SOURCES "require_container_is_permutation_negative.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
  ARGS --run_test=different_size_is_noticed
)
set_tests_properties (gspc_testing-require_container_is_permutation_negative-different_size_is_noticed
  PROPERTIES WILL_FAIL true
)

gspc_add_test (NAME gspc_testing-require_container_is_permutation_negative-different_content_is_noticed
  SOURCES "require_container_is_permutation_negative.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
  ARGS --run_test=different_content_is_noticed
)
set_tests_properties (gspc_testing-require_container_is_permutation_negative-different_content_is_noticed
  PROPERTIES WILL_FAIL true
)

gspc_add_test (NAME gspc_testing-require_exception
  SOURCES "require_exception.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)

gspc_add_test (NAME gspc_testing-require_type
  SOURCES "require_type.cpp"
  USE_BOOST
  LIBRARIES gspc_testing
)
