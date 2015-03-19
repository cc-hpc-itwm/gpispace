#define BOOST_TEST_MODULE FhgAssertTest
#include <boost/test/unit_test.hpp>

#include <fhg/assert.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

BOOST_AUTO_TEST_CASE(assert_true)
{
  fhg_assert(1 == 1, "assert_true test case");
}

BOOST_AUTO_TEST_CASE (assert_false)
{
  fhg::util::boost::test::require_exception<std::logic_error>
    ( []() { fhg_assert (1 == 0, "util_assert_false"); }
    , boost::format ("[%1%:%2%] assertion '%3%' failed%4%%5%.")
    % __FILE__
    % (__LINE__ - 3)
    % "1 == 0"
    % ": "
    % "util_assert_false"
    );
}

BOOST_AUTO_TEST_CASE(assert_true_empty_message)
{
  fhg_assert(1 == 1);
}

BOOST_AUTO_TEST_CASE (assert_false_empty_message)
{
  fhg::util::boost::test::require_exception<std::logic_error>
    ( []() { fhg_assert (1 == 0); }
    , boost::format ("[%1%:%2%] assertion '%3%' failed%4%%5%.")
    % __FILE__
    % (__LINE__ - 3)
    % "1 == 0"
    % ""
    % ""
    );
}
