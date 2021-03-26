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

#include <boost/test/unit_test.hpp>

#include <util-generic/cxx14/enum_hash.hpp>
#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_compiletime.hpp>

struct not_an_enum {};
enum an_enum {};
enum another_enum {};

template<typename T, typename = void>
  struct std_hash_exists : std::false_type {};

template<typename T>
  struct std_hash_exists<T, fhg::util::cxx17::void_t<decltype (std::hash<T>())>>
    : std::true_type {};


BOOST_AUTO_TEST_CASE (hash_not_blanket_generated_for_non_enums)
{
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE (!std_hash_exists<not_an_enum>{});
}

BOOST_AUTO_TEST_CASE (provides_std_hash_for_any_enum)
{
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE (std_hash_exists<an_enum>{});
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE (std_hash_exists<another_enum>{});
}

struct not_another_enum {};
enum enum_with_specific_std_hash {};

namespace
{
  std::size_t const random_hash_value
    (fhg::util::testing::random<std::size_t>{}());
}

namespace std
{
  template<> struct hash<not_another_enum>
  {
    std::size_t operator() (not_another_enum) const
    {
      return random_hash_value;
    }
  };
  template<> struct hash<enum_with_specific_std_hash>
  {
    std::size_t operator() (enum_with_specific_std_hash) const
    {
      return random_hash_value;
    }
  };
}

BOOST_AUTO_TEST_CASE (allows_specialization_of_std_hash)
{
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE (std_hash_exists<not_another_enum>{});
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE
    (std_hash_exists<enum_with_specific_std_hash>{});
}

BOOST_AUTO_TEST_CASE (really_uses_the_specialized_version)
{
  BOOST_REQUIRE_EQUAL ( std::hash<not_another_enum>{} (not_another_enum{})
                      , random_hash_value
                      );
  BOOST_REQUIRE_EQUAL ( std::hash<enum_with_specific_std_hash>{}
                          (enum_with_specific_std_hash{})
                      , random_hash_value
                      );
}

enum yet_another_enum : int
{
  one = 1,
  two = 2,
};

BOOST_AUTO_TEST_CASE (is_using_underlying_type_hash)
{
  BOOST_REQUIRE_EQUAL
    (std::hash<yet_another_enum>{} (one), std::hash<int>{} (1));
  BOOST_REQUIRE_EQUAL
    (std::hash<yet_another_enum>{} (two), std::hash<int>{} (2));
  BOOST_REQUIRE_NE (std::hash<yet_another_enum>{} (one), std::hash<int>{} (2));
  BOOST_REQUIRE_NE (std::hash<yet_another_enum>{} (two), std::hash<int>{} (1));
}
