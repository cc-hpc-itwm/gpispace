// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
