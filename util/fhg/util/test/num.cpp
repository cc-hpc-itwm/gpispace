// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE num
#include <boost/test/unit_test.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/error.hpp>

#include <string>
#include <iostream>

using fhg::util::parse::position;
namespace error = fhg::util::parse::error;

BOOST_AUTO_TEST_CASE (_ulong)
{
  using fhg::util::read_ulong;

  {
    const std::string inp ("23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23UL, read_ulong (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("23foo");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23UL, read_ulong (pos));
    BOOST_REQUIRE_EQUAL (pos.rest(), std::string ("foo"));
  }
  {
    const std::string inp ("-2");
    position pos (inp);

    BOOST_REQUIRE_THROW (read_ulong (pos), error::expected);
  }
}

BOOST_AUTO_TEST_CASE (_uint)
{
  using fhg::util::read_uint;

  const std::string inp ("23");
  position pos (inp);

  BOOST_REQUIRE_EQUAL (23U, read_uint (pos));
}

BOOST_AUTO_TEST_CASE (_long)
{
  using fhg::util::read_long;

  {
    const std::string inp ("23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23L, read_long (pos));
  }
  {
    const std::string inp ("-2");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2L, read_long (pos));
  }
}

BOOST_AUTO_TEST_CASE (_int)
{
  using fhg::util::read_int;

  {
    const std::string inp ("23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23, read_int (pos));
  }
  {
    const std::string inp ("-2");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2, read_int (pos));
  }
}

BOOST_AUTO_TEST_CASE (_double)
{
  using fhg::util::read_double;

  {
    const std::string inp ("23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23.0, read_double (pos));
  }
  {
    const std::string inp ("-2");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2.0, read_double (pos));
  }
  {
    const std::string inp ("-2e4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e4, read_double (pos));
  }
  {
    const std::string inp ("-2e+4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e4, read_double (pos));
  }
  {
    const std::string inp ("-2.e4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e4, read_double (pos));
  }
  {
    const std::string inp ("-2.0e4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e4, read_double (pos));
  }
  {
    const std::string inp ("-2e-4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e-4, read_double (pos));
  }
  {
    const std::string inp ("-0.125");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-0.125, read_double (pos));
  }
}

BOOST_AUTO_TEST_CASE (_float)
{
  using fhg::util::read_float;

  {
    const std::string inp ("23.125f");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (23.125f, read_float (pos));
  }
  {
    const std::string inp ("-2f");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2.0f, read_float (pos));
  }
  {
    const std::string inp ("-2e4f");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (-2e4f, read_float (pos));
  }
}

namespace
{
  template<typename T>
    void read_num_showed (const T& x)
  {
    using fhg::util::num_type;
    using fhg::util::read_num;

    const num_type n (x);

    {
      std::ostringstream oss;
      oss << n;
      position pos (oss.str());

      BOOST_REQUIRE_EQUAL (n, read_num (pos));
      BOOST_REQUIRE (pos.end());
    }
  }
}

BOOST_AUTO_TEST_CASE (_num)
{
  using fhg::util::num_type;
  using fhg::util::read_num;

  read_num_showed<float> (-2.e4f);
  read_num_showed<double> (-2.e4);
  read_num_showed<double> (0);
  read_num_showed<unsigned long> (0);
  read_num_showed<unsigned int> (0);
  read_num_showed<int> (0);
  read_num_showed<long> (0);

  {
    const std::string inp ("-2.125e4f");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-2.125e4f), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("-2.125e4");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-2.125e4), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("2.125e-1");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (2.125e-1), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (23), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("-23");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-23), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("23U");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (23U), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("-23U");
    position pos (inp);

    BOOST_REQUIRE_THROW (read_num (pos), error::signed_unsigned);
    BOOST_REQUIRE_EQUAL (pos.rest(), std::string ("U"));
  }
  {
    const std::string inp ("23L");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (23L), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("-23L");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-23L), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
  {
    const std::string inp ("23UL");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (23UL), read_num (pos));
    BOOST_REQUIRE (pos.end());
  }
}
