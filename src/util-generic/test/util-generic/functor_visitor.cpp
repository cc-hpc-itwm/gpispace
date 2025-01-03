// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/functor_visitor.hpp>

struct A {};
struct B {};
struct C {};

using A_or_B = ::boost::variant<A, B>;
using A_or_B_or_C = ::boost::variant<A, B, C>;

BOOST_AUTO_TEST_CASE (basic_use_case)
{
  auto visitor ( fhg::util::make_visitor<bool> ( [] (A) { return true; }
                                               , [] (B) { return false; }
                                               )
               );

  BOOST_REQUIRE (fhg::util::visit<bool> (A_or_B (A()), visitor));

  A_or_B const x {A()};
  BOOST_REQUIRE (::boost::apply_visitor (visitor, x));
}

BOOST_AUTO_TEST_CASE (convenience_visit)
{
  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B (A())
                                         , [] (A) { return true; }
                                         , [] (B) { return false; }
                                         )
                );
  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B (B())
                                         , [] (A) { return false; }
                                         , [] (B) { return true; }
                                         )
                );
}

BOOST_AUTO_TEST_CASE (exactly_one_function_is_called)
{
  int i (0);

  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B (A())
                                         , [&i] (A) { ++i; return true; }
                                         , [&i] (B) { ++i; return false; }
                                         )
                );

  BOOST_REQUIRE_EQUAL (i, 1);

  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B (A())
                                         , [&i] (B) { ++i; return false; }
                                         , [&i] (A) { ++i; return true; }
                                         )
                );

  BOOST_REQUIRE_EQUAL (i, 2);
}

BOOST_AUTO_TEST_CASE (coalescing_lambda)
{
  auto visitor ( fhg::util::make_visitor<bool>
                   ( [] (::boost::variant<A, B> const&) { return true; }
                   , [] (C) { return false; }
                   )
               );

  BOOST_REQUIRE (fhg::util::visit<bool> (A_or_B_or_C (A()), visitor));
  BOOST_REQUIRE (fhg::util::visit<bool> (A_or_B_or_C (B()), visitor));
  BOOST_REQUIRE (!fhg::util::visit<bool> (A_or_B_or_C (C()), visitor));
}

namespace
{
  template<typename T>
    struct is_T : private ::boost::static_visitor<bool>
  {
    bool operator() (T const&) const { return true; }
    template<typename U> bool operator() (U) const { return false; }
  };
}

BOOST_AUTO_TEST_CASE (visit_works_with_old_style_visitors_too)
{
  BOOST_REQUIRE (fhg::util::visit<bool> (A_or_B (A()), is_T<A>()));

  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B_or_C (A())
                                         , is_T<A>()
                                         , [] (C) { return false; }
                                         )
                );
  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B_or_C (C())
                                         , is_T<A>()
                                         , [] (C) { return true; }
                                         )
                );

  BOOST_REQUIRE ( fhg::util::visit<bool> ( A_or_B_or_C (A())
                                         , [] (C) { return false; }
                                         , is_T<A>()
                                         )
                );
}

namespace fhg
{
  namespace util
  {
    struct D
    {
      bool visited = false;
    };

    BOOST_AUTO_TEST_CASE (allows_modifying_visited_object)
    {
      ::boost::variant<D> x {D{}};

      fhg::util::visit<void> (x, [] (D& d) { d.visited = true; }, [] {});

      BOOST_REQUIRE_EQUAL (::boost::get<D> (x).visited, true);
    }
  }
}
