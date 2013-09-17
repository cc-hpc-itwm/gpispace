#define BOOST_TEST_MODULE ReadShowTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/show.hpp>

#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_CASE (test_read_boolalpha)
{
  std::string s_true ("true");
  bool b = fhg::util::read<bool>(s_true, std::ios_base::boolalpha);

  BOOST_REQUIRE_EQUAL (b, true);
}

BOOST_AUTO_TEST_CASE (test_show_boolalpha)
{
  std::string b = fhg::util::show (true, std::ios_base::boolalpha);

  BOOST_REQUIRE_EQUAL (b, "true");
}

BOOST_AUTO_TEST_CASE (test_show_bool)
{
  std::string b = fhg::util::show (true);

  BOOST_REQUIRE_EQUAL (b, "1");
}
