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

#include <util-generic/hash/combined_hash.hpp>

#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <functional>
#include <set>
#include <string>

struct pair
{
  std::string a;
  std::size_t b;
};

FHG_UTIL_MAKE_COMBINED_STD_HASH (::pair, p, p.a, p.b)

BOOST_AUTO_TEST_CASE (hash_value_has_at_least_some_entropy)
{
  std::set<std::size_t> hashes;

  for (std::size_t length (0); length < 100; ++length)
  {
    hashes.emplace
      (std::hash<pair>() (pair {std::string (length, 'X'), length}));
  }

  BOOST_REQUIRE_GT (hashes.size(), 80);
}
