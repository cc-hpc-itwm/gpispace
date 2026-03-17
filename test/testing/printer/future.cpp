#include <gspc/testing/printer/future.hpp>

#include <gspc/testing/printer/require_printed_as.hpp>

#include <boost/test/unit_test.hpp>

#include <future>

//! \note Yes, this test is dumb. It mainly is that way since it is
//! impossible to construct a scenario where std::future::wait_for
//! guaranteed returns deferred.
BOOST_AUTO_TEST_CASE (std_future_status_enum_is_printed)
{
  GSPC_TESTING_REQUIRE_PRINTED_AS ("ready", std::future_status::ready);
  GSPC_TESTING_REQUIRE_PRINTED_AS ("timeout", std::future_status::timeout);
  GSPC_TESTING_REQUIRE_PRINTED_AS ("deferred", std::future_status::deferred);
}
