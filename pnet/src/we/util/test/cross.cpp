// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_util_cross

#include <we/util/cross.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>

BOOST_AUTO_TEST_CASE (iterators_cons_list_empty)
{
  std::list<value::type> l;

  we::util::iterators_type i (l);

  BOOST_REQUIRE (i.pos_and_distance().first == l.begin());
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (0));
  BOOST_REQUIRE_EQUAL (i.end(), true);
}

BOOST_AUTO_TEST_CASE (iterators_cons_iterator_empty)
{
  std::list<value::type> l;

  we::util::iterators_type i (l.begin());

  BOOST_REQUIRE (i.pos_and_distance().first == l.begin());
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (0));
  BOOST_REQUIRE_EQUAL (i.end(), true);
}

BOOST_AUTO_TEST_CASE (iterators_cons_list)
{
  std::list<value::type> l;

  l.push_back (value::type (0L));

  we::util::iterators_type i (l);

  BOOST_REQUIRE (i.pos_and_distance().first == l.begin());
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (0));
  BOOST_REQUIRE_EQUAL (i.end(), false);
}

BOOST_AUTO_TEST_CASE (iterators_cons_iterator)
{
  std::list<value::type> l;

  l.push_back (value::type (0L));

  we::util::iterators_type i (l.begin());

  BOOST_REQUIRE (i.pos_and_distance().first == l.begin());
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (0));
  BOOST_REQUIRE_EQUAL (i.end(), false);
}

BOOST_AUTO_TEST_CASE (iterators_operator_plus)
{
  std::list<value::type> l;

  l.push_back (value::type (0L));

  we::util::iterators_type i (l);

  ++i;

  BOOST_REQUIRE (i.pos_and_distance().first == boost::next (l.begin()));
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (1));
  BOOST_REQUIRE_EQUAL (i.end(), true);
}

BOOST_AUTO_TEST_CASE (iterators_rewind)
{
  std::list<value::type> l;

  l.push_back (value::type (0L));

  we::util::iterators_type i (l);

  ++i;

  i.rewind();

  BOOST_REQUIRE (i.pos_and_distance().first == l.begin());
  BOOST_REQUIRE_EQUAL (i.pos_and_distance().second, std::size_t (0));
  BOOST_REQUIRE_EQUAL (i.end(), false);
}
