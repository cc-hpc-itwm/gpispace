#include <fhglog/format.hpp>

#define BOOST_TEST_MODULE format
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (percentage_escapes_percentage)
{
  BOOST_REQUIRE_EQUAL (fhg::log::format ("test %%", fhg::log::LogEvent()), "test %");
}

BOOST_AUTO_TEST_CASE (throw_on_invalid_escaped_sequence)
{
  BOOST_REQUIRE_THROW (fhg::log::check_format ("test %-"), std::runtime_error);
}

//! \todo This does not test
BOOST_AUTO_TEST_CASE (NOTEST_use_short_format)
{
  fhg::log::format (fhg::log::default_format::SHORT(), fhg::log::LogEvent());
}

//! \todo This should test all format flags!
