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

#include <util-generic/callable_signature.hpp>
#include <util-generic/testing/require_compiletime.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

namespace
{
  struct that
  {
    int fun (int, float)
    {
      ++_value;
      return _value;
    }

    int cfun() const
    {
      return _value;
    }

    int _value;
  };
}

BOOST_AUTO_TEST_CASE (returns_functor_callable_with_just_function_arguments)
{
  FHG_UTIL_TESTING_COMPILETIME_REQUIRE
    ( fhg::util::is_callable
        < decltype ( fhg::util::bind_this
                       (std::declval<that*>(), &that::fun)
                   )
        , int (int, float)
        >{}
    );
}

BOOST_AUTO_TEST_CASE (binds_this_as_a_reference)
{
  that t {0};
  auto fun (fhg::util::bind_this (&t, &that::fun));
  BOOST_REQUIRE_EQUAL (fun (int(), float()), 1);

  t._value = 120;
  BOOST_REQUIRE_EQUAL (fun (int(), float()), 121);
}

BOOST_AUTO_TEST_CASE (const_this)
{
  that const t {120};
  auto cfun (fhg::util::bind_this (&t, &that::cfun));
  BOOST_REQUIRE_EQUAL (cfun(), 120);
}

BOOST_AUTO_TEST_CASE (const_memfn)
{
  {
    that const t {120};
    auto cfun (fhg::util::bind_this (&t, &that::cfun));
    BOOST_REQUIRE_EQUAL (cfun(), 120);
  }
  {
    that t {120};
    auto cfun (fhg::util::bind_this (&t, &that::cfun));
    BOOST_REQUIRE_EQUAL (cfun(), 120);
    t._value = 1;
    BOOST_REQUIRE_EQUAL (cfun(), 1);
  }
}
