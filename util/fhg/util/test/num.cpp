// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE num
#include <boost/test/unit_test.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/num/show.hpp>
#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>

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
    BOOST_REQUIRE_EQUAL (fhg::util::parse::require::rest (pos), std::string ("foo"));
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
      const std::string inp (oss.str());
      position pos (inp);

      BOOST_CHECK_EQUAL (n, read_num (pos));
      BOOST_CHECK (pos.end());
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
    const std::string inp ("-1U");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-1U), read_num (pos));
    BOOST_REQUIRE (pos.end());
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

  {
    const std::string inp ("23UL 44.0f 23U 23L 23 44.0 rest");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (23UL), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (num_type (44.0f), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (num_type (23U), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (num_type (23L), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (num_type (23), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (num_type (44.0), read_num (pos));
    fhg::util::parse::require::skip_spaces (pos);
    BOOST_REQUIRE_EQUAL (fhg::util::parse::require::rest (pos), "rest");
  }
}

BOOST_AUTO_TEST_CASE (unexpected_digit)
{
#define CHECK(s,T,f)                                                    \
  {                                                                     \
    const std::string inp (s);                                          \
    position p (inp);                                                   \
                                                                        \
    BOOST_CHECK_THROW ( fhg::util::read_ ## f (p)                       \
                      , error::unexpected_digit<T>                      \
                      );                                                \
  }

  CHECK ("4294967297", unsigned int, uint);
  CHECK ("42949672960", unsigned int, uint);
  CHECK ("0x100000000", unsigned int, uint);
  CHECK ("0x80000000", int, int);
  {
    const std::string inp ("-0x80000000");
    position p (inp);

    BOOST_CHECK_EQUAL (fhg::util::read_int (p), -0x80000000);
  }
  CHECK ("-0x80000001", int, int);

#undef CHECK
}

BOOST_AUTO_TEST_CASE (limits)
{
  using fhg::util::num_type;
  using fhg::util::read_num;

  {
    const std::string inp ("9223372036854775807L");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (9223372036854775807L), read_num (pos));
  }
  {
    const std::string inp ("-9223372036854775807L");
    position pos (inp);

    BOOST_REQUIRE_EQUAL (num_type (-9223372036854775807L), read_num (pos));
  }
  {
    const std::string inp ("9223372036854775808L");
    position pos (inp);

    typedef error::value_too_big<unsigned long, long> error_type;

    BOOST_REQUIRE_THROW (read_num (pos), error_type);
  }
  {
    const std::string inp ("-9223372036854775808L");
    position pos (inp);

    typedef error::value_too_big<unsigned long, long> error_type;

    BOOST_REQUIRE_THROW (read_num (pos), error_type);
  }
  {
    const double d ( 2.0
                   * static_cast<double> (std::numeric_limits<float>::max())
                   );
    std::ostringstream oss;
    oss << d << "f";
    const std::string inp (oss.str());
    position pos (inp);

    typedef error::value_too_big<double, float> error_type;

    BOOST_REQUIRE_THROW (read_num (pos), error_type);
  }
}
