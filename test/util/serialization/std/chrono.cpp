#include <gspc/util/serialization/std/chrono.hpp>
#include <gspc/testing/printer/chrono.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_serialized_to_id.hpp>

#include <boost/test/unit_test.hpp>

#define RANDOM_DURATION_REQUIRE_SERIALIZED_ID(type_...)                 \
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID                             \
    ({(gspc::testing::random<type_::rep>{}())}, type_)

BOOST_AUTO_TEST_CASE (can_handle_standard_durations)
{
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::nanoseconds);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::microseconds);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::milliseconds);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::seconds);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::minutes);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::hours);
}

BOOST_AUTO_TEST_CASE (can_handle_custom_durations)
{
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::duration<int, std::ratio<1, 100000>>);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::duration<int, std::centi>);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::duration<float, std::ratio<12096, 10000>>);
  RANDOM_DURATION_REQUIRE_SERIALIZED_ID
    (std::chrono::duration<float, std::ratio<3155, 1000>>);
}

BOOST_AUTO_TEST_CASE (can_handle_time_poitns)
{
  auto current_system (std::chrono::system_clock::now());
  auto yesterday (current_system - std::chrono::hours (24));
  auto current_hires (std::chrono::high_resolution_clock::now());

  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({current_system}, decltype (current_system));
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({yesterday}, decltype (yesterday));
  GSPC_TESTING_REQUIRE_SERIALIZED_TO_ID
    ({current_hires}, decltype (current_hires));
}
