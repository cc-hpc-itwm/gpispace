// Copyright (C) 2011-2016,2018-2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/generic.hpp>
#include <gspc/testing/printer/tuple.hpp>

#include <gspc/we/type/bytearray.hpp>

#include <gspc/testing/random/string.hpp>

#include <inttypes.h>

BOOST_AUTO_TEST_CASE (ba_char)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  const gspc::we::type::bytearray x (buf, 10);

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (x.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (x.copy (buf, 12), 10);
}
BOOST_AUTO_TEST_CASE (ba_uint64_t)
{
  uint64_t v (1UL << 63);

  const gspc::we::type::bytearray x (&v);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), sizeof (uint64_t));
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}
BOOST_AUTO_TEST_CASE (ba_convert)
{
  uint64_t v (1UL << 63);

  const gspc::we::type::bytearray x (&v);

  BOOST_REQUIRE_EQUAL (sizeof (uint64_t), 8);

  char buf[8];

  BOOST_CHECK_EQUAL (x.copy (buf, 8), 8);
  BOOST_CHECK_EQUAL (buf[0], 0);
  BOOST_CHECK_EQUAL (buf[1], 0);
  BOOST_CHECK_EQUAL (buf[2], 0);
  BOOST_CHECK_EQUAL (buf[3], 0);
  BOOST_CHECK_EQUAL (buf[4], 0);
  BOOST_CHECK_EQUAL (buf[5], 0);
  BOOST_CHECK_EQUAL (buf[6], 0);
  BOOST_CHECK_EQUAL (buf[7], -128);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), 8);
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}

BOOST_AUTO_TEST_CASE (ba_to_string_after_ctor_string_is_id)
{
  std::string const s (gspc::testing::random_string());

  BOOST_REQUIRE_EQUAL (gspc::we::type::bytearray (s).to_string(), s);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_ba)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  gspc::we::type::bytearray y;

  {
    const gspc::we::type::bytearray x (buf, 10);

    y = x;
  }

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (y.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (y.copy (buf, 12), 10);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_alien)
{
  gspc::we::type::bytearray ba;

  ba = 1.0f;

  float f;

  BOOST_CHECK_EQUAL (ba.copy (&f), sizeof (float));
  BOOST_CHECK_EQUAL (f, 1.0f);
}

namespace
{
  template<typename T> void use_ctor_and_copy_and_require_equal (T x)
  {
    gspc::we::type::bytearray ba (x);
    T y;
    ba.copy (&y);
    BOOST_REQUIRE_EQUAL (x, y);
  }

  struct Pod
  {
    int x;
    int y;
  };
  static_assert (std::is_trivially_copyable_v<Pod>);
  bool operator== (Pod const& lhs, Pod const& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
}

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (Pod, os, pod)
{
  os << pod.x << ", " << pod.y;
}

namespace
{
  struct Tuple
  {
    bool _bool;
    double _double;
    char _char;
  };
  bool operator== (Tuple const& lhs, Tuple const& rhs)
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x._bool, x._double, x._char);
        }
      };
    return essence (lhs) == essence (rhs);
  }
}

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (Tuple, os, tuple)
{
  os << "<" << tuple._bool << ", " << tuple._double << ", " << tuple._char << ">";
}

BOOST_AUTO_TEST_CASE (ctor_from_T)
{
  use_ctor_and_copy_and_require_equal (1.0f);
  use_ctor_and_copy_and_require_equal (Pod {1, 15});
  use_ctor_and_copy_and_require_equal (Tuple {false, 8.0, 'a'});
}
