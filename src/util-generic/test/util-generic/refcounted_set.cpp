// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/refcounted_set.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/test_case.hpp>

#include <boost/test/unit_test.hpp>

#include <set>
#include <string>

FHG_UTIL_TESTING_TEMPLATED_CASE (refcounted_set, int, std::string)
{
  fhg::util::refcounted_set<T> s;

  fhg::util::testing::unique_random<T> random;

  T const a {random()};
  T const b {random()};

  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.emplace (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);

  s.emplace (a);
  s.emplace (a);
  s.emplace (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.emplace (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), true);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 2);

  s.erase (a);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), true);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 1);

  s.erase (b);
  BOOST_REQUIRE_EQUAL (s.contains (a), false);
  BOOST_REQUIRE_EQUAL (s.contains (b), false);
  BOOST_REQUIRE_EQUAL (static_cast<std::set<T>> (s).size(), 0);
}

BOOST_AUTO_TEST_CASE (emplace_works_with_temporaries)
{
  fhg::util::refcounted_set<std::string> s;

  std::string const content (fhg::util::testing::random<std::string>{}());
  //! \note explicitly make temporary
  s.emplace (std::string (content));
  BOOST_REQUIRE (s.contains (content));
  s.erase (content);
  BOOST_REQUIRE (!s.contains (content));
}
