// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/tuple.hpp>

#include <we/type/bytearray.hpp>

#include <util-generic/testing/random/string.hpp>

#include <inttypes.h>

BOOST_AUTO_TEST_CASE (ba_char)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  const we::type::bytearray x (buf, 10);

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (x.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (x.copy (buf, 12), 10);
}
BOOST_AUTO_TEST_CASE (ba_uint64_t)
{
  uint64_t v (1UL << 63);

  const we::type::bytearray x (&v);

  v = 0;

  BOOST_CHECK_EQUAL (x.copy (&v), sizeof (uint64_t));
  BOOST_CHECK_EQUAL (v, (1UL << 63));
}
BOOST_AUTO_TEST_CASE (ba_convert)
{
  uint64_t v (1UL << 63);

  const we::type::bytearray x (&v);

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
  std::string const s (fhg::util::testing::random_string());

  BOOST_REQUIRE_EQUAL (we::type::bytearray (s).to_string(), s);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_ba)
{
  char buf[10];

  for (int i (0); i < 10; ++i)
    {
      buf[i] = i + 'a';
    }

  we::type::bytearray y;

  {
    const we::type::bytearray x (buf, 10);

    y = x;
  }

  std::fill (buf, buf + 10, 0);

  BOOST_CHECK_EQUAL (y.copy (buf, 10), 10);
  BOOST_CHECK_EQUAL (std::string (buf, buf + 10), "abcdefghij");
  BOOST_CHECK_EQUAL (y.copy (buf, 12), 10);
}

BOOST_AUTO_TEST_CASE (ba_assign_from_alien)
{
  we::type::bytearray ba;

  ba = 1.0f;

  float f;

  BOOST_CHECK_EQUAL (ba.copy (&f), sizeof (float));
  BOOST_CHECK_EQUAL (f, 1.0f);
}

namespace
{
  template<typename T> void use_ctor_and_copy_and_require_equal (T x)
  {
    we::type::bytearray ba (x);
    T y;
    ba.copy (&y);
    BOOST_REQUIRE_EQUAL (x, y);
  }

  struct Pod
  {
    int x;
    int y;
  };
  bool operator== (Pod const& lhs, Pod const& rhs)
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
}

FHG_BOOST_TEST_LOG_VALUE_PRINTER (Pod, os, pod)
{
  os << pod.x << ", " << pod.y;
}

BOOST_AUTO_TEST_CASE (ctor_from_T)
{
  use_ctor_and_copy_and_require_equal (1.0f);
  use_ctor_and_copy_and_require_equal (Pod {1, 15});
  use_ctor_and_copy_and_require_equal (std::make_tuple (false, 8.0, 'a'));
}
