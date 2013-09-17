#define BOOST_TEST_MODULE FhgAssertTest
#include <boost/test/unit_test.hpp>

#include <cassert>
#include <fhg/assert.hpp>
#include <fhg/assertion_failed.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE(assert_true)
{
  try
  {
    fhg_assert(1 == 1, "assert_true test case");
  }
  catch (fhg::assertion_failed const &)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE(assert_false)
{
  try
  {
    fhg_assert(1 == 0, "assert_false test case");
    throw std::runtime_error("assert_false test case did not throw!");
  }
  catch (fhg::assertion_failed const & af)
  {
    std::cerr << af.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(assert_true_empty_message)
{
  try
  {
    fhg_assert(1 == 1);
  }
  catch (fhg::assertion_failed const &)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE(assert_false_empty_message)
{
  try
  {
    fhg_assert(1 == 0);
    throw std::runtime_error("assert_false_empty_message did not throw!");
  }
  catch (fhg::assertion_failed const & af)
  {
    std::cerr << af.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(replace_legacy_assert_true)
{
  try
  {
    assert (1==1);
  }
  catch (fhg::assertion_failed const &)
  {
    throw;
  }
}

BOOST_AUTO_TEST_CASE(replace_legacy_assert_false)
{
  try
  {
    assert(1 == 0);
    throw std::runtime_error("assert_false_empty_message did not throw!");
  }
  catch (fhg::assertion_failed const & af)
  {
    std::cerr << af.what() << std::endl;
  }
}
