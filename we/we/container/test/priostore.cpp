// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_container_priostore

#include <we/container/priostore.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (basic)
{
  we::container::priority_store ps;

  ps.insert (0);

  BOOST_REQUIRE_EQUAL (ps.elem (0), true);
  BOOST_REQUIRE_EQUAL (ps.elem (1), false);
  BOOST_REQUIRE_EQUAL (ps.get_priority (0), petri_net::priority_type());
  BOOST_REQUIRE_EQUAL (ps.get_priority (1), petri_net::priority_type());
}
