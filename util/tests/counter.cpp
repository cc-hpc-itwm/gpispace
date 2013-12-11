#define BOOST_TEST_MODULE UtilCounter
#include <boost/test/unit_test.hpp>

#include <fhg/util/counter.hpp>

#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_CASE ( test_default_ctor )
{
  fhg::util::counter<int> counter;

  {
    const int actual (counter.next());
    BOOST_REQUIRE_EQUAL (0, actual);
  }
}

BOOST_AUTO_TEST_CASE ( test_initial_value )
{
  fhg::util::counter<int> counter (10);

  {
    const int actual (counter.next());
    BOOST_REQUIRE_EQUAL (10, actual);
  }
}

BOOST_AUTO_TEST_CASE ( test_cast_operator )
{
  fhg::util::counter<int> counter;

  {
    const int actual (counter);
    BOOST_REQUIRE_EQUAL (0, actual);
  }
  {
    const int actual (counter);
    BOOST_REQUIRE_EQUAL (1, actual);
  }
}

BOOST_AUTO_TEST_CASE ( test_next )
{
  fhg::util::counter<int> counter;

  {
    const int actual (counter.next());
    BOOST_REQUIRE_EQUAL (0, actual);
  }
  {
    const int actual (counter.next());
    BOOST_REQUIRE_EQUAL (1, actual);
  }
}

BOOST_AUTO_TEST_CASE ( test_multiple_counters )
{
  fhg::util::counter<int> counter_a;
  fhg::util::counter<int> counter_b;

  {
    const int actual (counter_a);
    BOOST_REQUIRE_EQUAL (0, actual);
  }
  {
    const int actual (counter_b);
    BOOST_REQUIRE_EQUAL (0, actual);
  }
  {
    const int actual (counter_b);
    BOOST_REQUIRE_EQUAL (1, actual);
  }
  {
    const int actual (counter_a);
    BOOST_REQUIRE_EQUAL (1, actual);
  }
}

#include "boost/mpl/equal.hpp"

BOOST_AUTO_TEST_CASE ( test_value_type )
{
  typedef fhg::util::counter<float> float_counter;
  typedef boost::mpl::equal<float_counter::value_type,float>::type is_equal;

  BOOST_REQUIRE_EQUAL (true, is_equal::value);
}
