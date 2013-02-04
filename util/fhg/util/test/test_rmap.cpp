// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE rmap
#include <boost/test/unit_test.hpp>

#include <fhg/util/rmap.hpp>

#include <boost/unordered_map.hpp>

#include <string>
#include <iostream>
#include <sstream>

BOOST_AUTO_TEST_CASE (basic)
{
  typedef fhg::util::rmap::traits<std::string,long> rmap_traits;

  rmap_traits::rmap_type rmap;
  rmap_traits::keys_type keys_empty;

  rmap_traits::keys_type keys_a;
  keys_a.push_back ("a");

  rmap_traits::keys_type keys_b;
  keys_b.push_back ("b");

  rmap_traits::keys_type keys_bb;
  keys_bb.push_back ("b");
  keys_bb.push_back ("b");

  const std::string key_c ("c");

  BOOST_REQUIRE_EQUAL (rmap.bind (keys_empty, 1), 1);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 1);
  }


  BOOST_REQUIRE_EQUAL (rmap.bind (keys_empty, 2), 2);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 2);
  }


  BOOST_REQUIRE_EQUAL (rmap.bind (keys_a, 3), 3);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_a));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 3);
  }


  BOOST_REQUIRE_EQUAL (rmap.bind (keys_b, 4), 4);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_a));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 3);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_b));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 4);
  }


  BOOST_REQUIRE_EQUAL (rmap.bind (keys_bb, 4), 4);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_a));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 3);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_b));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_bb));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 4);
  }


  BOOST_REQUIRE_EQUAL (rmap.bind (keys_bb, 5), 5);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_a));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 3);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_b));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_bb));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 5);
  }

  BOOST_REQUIRE_EQUAL (rmap.bind (key_c, 5), 5);
  {
    rmap_traits::query_result_type res (rmap.value (keys_empty));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_a));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 3);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_b));

    BOOST_REQUIRE (!res);
  }
  {
    rmap_traits::query_result_type res (rmap.value (keys_bb));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 5);
  }
  {
    rmap_traits::query_result_type res (rmap.value (key_c));

    BOOST_REQUIRE (res);
    BOOST_REQUIRE_EQUAL (*res, 5);
  }

  rmap_traits::keys_type keys_ba;
  keys_ba.push_back ("b");
  keys_ba.push_back ("a");
  BOOST_REQUIRE_EQUAL (rmap.bind (keys_ba, 19), 19);

  rmap_traits::keys_type keys_bc;
  keys_bc.push_back ("b");
  keys_bc.push_back ("c");
  BOOST_REQUIRE_EQUAL (rmap.bind (keys_bc, 17), 17);

  rmap_traits::keys_type keys_bba;
  keys_bba.push_back ("b");
  keys_bba.push_back ("b");
  keys_bba.push_back ("a");
  BOOST_REQUIRE_EQUAL (rmap.bind (keys_bba, 23), 23);

  rmap_traits::keys_type keys_bbbb;
  keys_bbbb.push_back ("b");
  keys_bbbb.push_back ("b");
  keys_bbbb.push_back ("b");
  keys_bbbb.push_back ("b");
  BOOST_REQUIRE_EQUAL (rmap.bind (keys_bbbb, 42), 42);

  std::ostringstream oss_cmp;
  oss_cmp << "a:" << std::endl;
  oss_cmp << " | 3" << std::endl;
  oss_cmp << "b:" << std::endl;
  oss_cmp << " | a:" << std::endl;
  oss_cmp << " |  | 19" << std::endl;
  oss_cmp << " | b:" << std::endl;
  oss_cmp << " |  | a:" << std::endl;
  oss_cmp << " |  |  | 23" << std::endl;
  oss_cmp << " |  | b:" << std::endl;
  oss_cmp << " |  |  | b:" << std::endl;
  oss_cmp << " |  |  |  | 42" << std::endl;
  oss_cmp << " | c:" << std::endl;
  oss_cmp << " |  | 17" << std::endl;
  oss_cmp << "c:" << std::endl;
  oss_cmp << " | 5" << std::endl;

  std::ostringstream oss;
  oss << rmap;

  BOOST_REQUIRE_EQUAL (oss_cmp.str(), oss.str());
}
