option (BUILD_MILLER_RABIN "Build the miller-rabin prime test example
  implementation. Requires GMP." ON
)

set (GMP_PREFIX "" CACHE PATH "The path to the prefix of a GMP installation")
if ("${GMP_PREFIX}" STREQUAL "" AND NOT "$ENV{GMP_PREFIX}" STREQUAL "")
  set (GMP_PREFIX "$ENV{GMP_PREFIX}")
endif()

find_path (GMP_INCLUDE_DIR gmpxx.h
  PATHS ${GMP_PREFIX}/include
)
find_library (GMP_LIBRARY_gmp NAMES gmp
  PATHS ${GMP_PREFIX}/lib
)
find_library (GMP_LIBRARY_gmpxx NAMES gmpxx
  PATHS ${GMP_PREFIX}/lib
)

if (NOT (GMP_INCLUDE_DIR AND GMP_LIBRARY_gmp AND GMP_LIBRARY_gmpxx))
  message (WARNING "Could not find GMP, skipping miller-rabin implementation."
    "(GMP_INCLUDE_DIR=${GMP_INCLUDE_DIR}, GMP_LIBRARY_gmp=${GMP_LIBRARY_gmp}, "
    "GMP_LIBRARY_gmpxx=${GMP_LIBRARY_gmpxx})"
  )
  set (BUILD_MILLER_RABIN OFF)
endif()

if (BUILD_MILLER_RABIN)
  add_library (gmp INTERFACE)
  target_link_libraries (gmp
    INTERFACE "${GMP_LIBRARY_gmpxx}"
    INTERFACE "${GMP_LIBRARY_gmp}")
  target_include_directories (gmp INTERFACE "${GMP_INCLUDE_DIR}")

  add_library (util STATIC "util.cpp")
  target_link_libraries (util
    PUBLIC gmp
    PUBLIC GPISpace::workflow_development)
  set_target_properties (util PROPERTIES POSITION_INDEPENDENT_CODE ON)

  add_executable (miller-rabin-utils.test "util-test.cpp")
  target_compile_definitions (miller-rabin-utils.test PRIVATE
    "BOOST_TEST_MODULE=\"miller-rabin-utils\""
  )
  target_link_libraries (miller-rabin-utils.test
    PRIVATE util
    PRIVATE Boost::unit_test_framework)
  add_test (NAME miller-rabin-utils COMMAND
    $<TARGET_FILE:miller-rabin-utils.test>
  )

  swh_add_implementation_module (NAME miller-rabin
    SOURCES "miller-rabin.cpp"
    LIBRARIES PRIVATE util
  )

  swh_add_implementation_exe (NAME miller-rabin
    SOURCES "run.cpp"
    LIBRARIES PRIVATE util
  )
endif()

install (FILES "user-example/generate_user_data.cpp"
               "user-example/Makefile"
               "user-example/miller-rabin.cpp"
               "user-example/README"
               "user-example/show_result.cpp"
               "user-example/util.cpp"
               "user-example/util.hpp"
  DESTINATION "example/miller-rabin"
)
