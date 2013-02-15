// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_container_priostore

#include <we/container/priostore.hpp>

#include <boost/random.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (insert_and_elem)
{
  we::container::priority_store ps;

  BOOST_REQUIRE_EQUAL (ps.elem (0), false);

  ps.insert (0);

  BOOST_REQUIRE_EQUAL (ps.elem (0), true);
}

BOOST_AUTO_TEST_CASE (erase)
{
  we::container::priority_store ps;

  ps.insert (0);
  ps.erase (0);

  BOOST_REQUIRE_EQUAL (ps.elem (0), false);
}

BOOST_AUTO_TEST_CASE (empty)
{
  we::container::priority_store ps;

  BOOST_REQUIRE_EQUAL (ps.empty(), true);

  ps.insert (0);

  BOOST_REQUIRE_EQUAL (ps.empty(), false);

  ps.erase (0);

  BOOST_REQUIRE_EQUAL (ps.empty(), true);
}

BOOST_AUTO_TEST_CASE (get_default_priority)
{
  we::container::priority_store ps;

  BOOST_REQUIRE_EQUAL (ps.get_priority (0), petri_net::priority_type());
}

BOOST_AUTO_TEST_CASE (get_set_priority)
{
  we::container::priority_store ps;

  ps.set_priority (0, 1);

  BOOST_REQUIRE_EQUAL (ps.get_priority (0), petri_net::priority_type (1));
}

BOOST_AUTO_TEST_CASE (erase_priority)
{
  we::container::priority_store ps;

  ps.set_priority (0, 1);
  ps.erase_priority (0);

  BOOST_REQUIRE_EQUAL (ps.get_priority (0), petri_net::priority_type());
}

BOOST_AUTO_TEST_CASE (extract_random_with_highest_priority)
{
  we::container::priority_store ps;

  ps.insert (0);
  ps.insert (1);
  ps.set_priority (0, 1);
  ps.set_priority (1, 2);

  boost::mt19937 engine;

  BOOST_REQUIRE_EQUAL (ps.random (engine), petri_net::transition_id_type (1));
}
