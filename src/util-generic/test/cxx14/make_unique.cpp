// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/require_type.hpp>

#include <memory>

using fhg::util::cxx14::make_unique;

enum ctor_type
{
  default_ctor,
  size_t_ctor,
  two_size_t_ctor,
  int_ctor,
};
struct multiple_ctors
{
  multiple_ctors() : invoked (default_ctor) {}
  multiple_ctors (std::size_t) : invoked (size_t_ctor) {}
  multiple_ctors (std::size_t, std::size_t) : invoked (two_size_t_ctor) {}
  multiple_ctors (int) : invoked (int_ctor) {}

  ctor_type invoked;
};

BOOST_AUTO_TEST_CASE (type_can_have_different_ctors)
{
  BOOST_REQUIRE_EQUAL (make_unique<multiple_ctors>()->invoked, default_ctor);
  BOOST_REQUIRE_EQUAL (make_unique<multiple_ctors> (int())->invoked, int_ctor);
  BOOST_REQUIRE_EQUAL
    (make_unique<multiple_ctors> (std::size_t())->invoked, size_t_ctor);
  BOOST_REQUIRE_EQUAL
    ( make_unique<multiple_ctors> (std::size_t(), std::size_t())->invoked
    , two_size_t_ctor
    );
}

BOOST_AUTO_TEST_CASE
  (array_overload_uses_default_ctor_even_if_ctor_of_size_t_exists)
{
  BOOST_REQUIRE_EQUAL
    (make_unique<multiple_ctors> (std::size_t (1))->invoked, size_t_ctor);
  BOOST_REQUIRE_EQUAL
    (make_unique<multiple_ctors[]> (std::size_t (1))[0].invoked, default_ctor);
}

struct no_default_ctor
{
  no_default_ctor() = delete;
  no_default_ctor (int) {}
};
BOOST_AUTO_TEST_CASE (no_default_ctor_required)
{
  make_unique<no_default_ctor> (1);
}

struct no_move_or_copy
{
  no_move_or_copy() = default;
  no_move_or_copy (no_move_or_copy const&) = delete;
  no_move_or_copy (no_move_or_copy&&) = delete;
  no_move_or_copy& operator= (no_move_or_copy const&) = delete;
  no_move_or_copy& operator= (no_move_or_copy&&) = delete;
};
BOOST_AUTO_TEST_CASE (no_move_or_copy_required)
{
  make_unique<no_move_or_copy>();
}

template<typename U>
  struct check_forwarding
{
  template<typename T> check_forwarding (T&& x)
  {
    FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL (decltype (x), U);
  }
};
BOOST_AUTO_TEST_CASE (arguments_are_perfect_forwarded)
{
  make_unique<check_forwarding<std::string&&>> (std::string());

  int reference{};
  make_unique<check_forwarding<int&>> (reference);

  int const const_reference{};
  make_unique<check_forwarding<int const&>> (const_reference);
}
