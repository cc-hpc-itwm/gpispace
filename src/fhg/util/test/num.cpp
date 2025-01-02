// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/num/show.hpp>
#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/require.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <FMT/std/variant.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <iostream>
#include <string>

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

namespace
{
  template<typename T>
    void read_num_showed (T const& x)
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
  do                                                                    \
  {                                                                     \
    const std::string inp (s);                                          \
    position p (inp);                                            \
                                                                        \
    BOOST_CHECK_THROW ( fhg::util::read_ ## f (p)                       \
                      , error::unexpected_digit<T>                      \
                      );                                                \
  } while (false)

  CHECK ("4294967297", unsigned int, uint);
  CHECK ("42949672960", unsigned int, uint);
  CHECK ("0x100000000", unsigned int, uint);

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

    using error_type = error::value_too_big<unsigned long, long>;

    BOOST_REQUIRE_THROW (read_num (pos), error_type);
  }
  {
    const std::string inp ("-9223372036854775808L");
    position pos (inp);

    using error_type = error::value_too_big<unsigned long, long>;

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

    using error_type = error::value_too_big<double, float>;

    BOOST_REQUIRE_THROW (read_num (pos), error_type);
  }
}

namespace fhg
{
  namespace util
  {
    // When reading floats Implementation casted float_max to ul_max,
    // which overflows and thus is undefined behavior. In the observed
    // instances it ended up comparing against 0, failing because 1 > 0.
    BOOST_DATA_TEST_CASE (issue_896_limits_checked_incorrectly_narrow_cast
     , std::vector<std::string> ({"1f", "+1f", "-1f", "1F", "+1F", "-1F"})
     ^ std::vector<num_type>    ({ 1.f,   1.f,  -1.f,  1.f,   1.f,  -1.f})
     , input
     , expected
      )
    {
      position pos (input);
      num_type result;
      BOOST_REQUIRE_NO_THROW (result = read_num (pos));
      BOOST_REQUIRE_EQUAL (result, expected);
    }
  }
}
