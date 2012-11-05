// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE Either
#include <boost/test/unit_test.hpp>

#include <fhg/util/either.hpp>

#include <string>
#include <iostream>

typedef fhg::util::either::type<std::string, int> string_or_int_type;

BOOST_AUTO_TEST_CASE (ctor_left__left_right)
{
  string_or_int_type e ("Hi");

  BOOST_REQUIRE (e.is_left());
  BOOST_REQUIRE_EQUAL (e.left(), "Hi");

  e = "Beep";

  BOOST_REQUIRE (e.is_left());
  BOOST_REQUIRE_EQUAL (e.left(), "Beep");

  e = 42;

  BOOST_REQUIRE (e.is_right());
  BOOST_REQUIRE_EQUAL (e.right(), 42);
}

BOOST_AUTO_TEST_CASE (ctor_right)
{
  string_or_int_type e (42);

  BOOST_REQUIRE (e.is_right());
  BOOST_REQUIRE_EQUAL (e.right(), 42);
}

BOOST_AUTO_TEST_CASE (fail_getting_right_if_left)
{
  string_or_int_type e ("left");

  BOOST_REQUIRE (e.is_left());

  try
  {
    std::cout << e.right() << std::endl;

    BOOST_FAIL ("should throw");
  }
  catch (const boost::bad_get&)
  {
  }
  catch (...)
  {
    BOOST_FAIL ("should throw boost::bad_get");
  }
}
