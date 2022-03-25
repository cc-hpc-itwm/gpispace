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

#include <util-generic/testing/random.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <cmath>

BOOST_AUTO_TEST_CASE (char_of_empty)
{
  fhg::util::testing::require_exception
    ( [] { fhg::util::testing::random<char>{} (std::string()); }
    , std::logic_error ("random<char>: precondition failed: !chars.empty()")
    );
}

BOOST_AUTO_TEST_CASE (char_of_one)
{
  BOOST_REQUIRE_EQUAL (fhg::util::testing::random<char>{} ("c"), 'c');
}

BOOST_AUTO_TEST_CASE (char_of_some)
{
  std::string const chars ("abcdefgh0123");

  for (int i (0); i < 1000; ++i)
  {
    BOOST_REQUIRE_NE
      ( chars.find (fhg::util::testing::random<char>{} (chars))
      , std::string::npos
      );
  }
}

BOOST_AUTO_TEST_CASE (char_of_distribution)
{
  std::string const chars ("011");
  std::size_t count[2] = {0,0};
  std::size_t len (0);

  while (count[0] == 0 || len < (1UL << 15))
  {
    ++count[fhg::util::testing::random<char>{} (chars) - '0'];
    ++len;
  }

  double const ratio (double (count[1]) / double (count[0]));

  BOOST_REQUIRE_LT (std::fabs (ratio - 2.0), 0.1);
}

BOOST_AUTO_TEST_CASE (string_of_empty)
{
  fhg::util::testing::require_exception
    ( [] { fhg::util::testing::random<std::string>{} (std::string()); }
    , std::logic_error ("random<std::string>: precondition failed: !chars.empty()")
    );
}

BOOST_AUTO_TEST_CASE (string_of_one)
{
  std::string const chars ("c");

  for (char const& c : fhg::util::testing::random<std::string>{} (chars))
  {
    BOOST_REQUIRE_NE (chars.find (c), std::string::npos);
  }
}

BOOST_AUTO_TEST_CASE (string_of_some)
{
  std::string const chars ("abcdefgh0123");

  for (char const& c : fhg::util::testing::random<std::string>{} (chars))
  {
    BOOST_REQUIRE_NE (chars.find (c), std::string::npos);
  }
}

BOOST_AUTO_TEST_CASE (string_of_distribution)
{
  std::string const chars ("011");
  std::size_t count[2] = {0,0};
  std::size_t len (0);

  while (count[0] == 0 || len < (1UL << 15))
  {
    std::string const s (fhg::util::testing::random<std::string>{} (chars));

    len += s.size();

    for (char const& c : s)
    {
      ++count[c - '0'];
    }
  }

  double const ratio (double (count[1]) / double (count[0]));

  BOOST_REQUIRE_LT (std::fabs (ratio - 2.0), 0.1);
}

BOOST_AUTO_TEST_CASE (string_without)
{
  auto const except ( fhg::util::testing::random<std::string>{}
                        ("0123456789abcdefghijklmnopqrstuvwyxz${}")
                    );

  using random_string = fhg::util::testing::random<std::string>;

  for (char const& c : random_string{} (random_string::except (except)))
  {
    BOOST_REQUIRE_EQUAL (except.find (c), std::string::npos);
  }
}
