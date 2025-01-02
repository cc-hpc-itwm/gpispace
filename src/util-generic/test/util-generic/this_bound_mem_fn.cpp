// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
