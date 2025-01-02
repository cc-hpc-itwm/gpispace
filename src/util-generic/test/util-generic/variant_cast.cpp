// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/variant_cast.hpp>

#include <boost/mpl/distance.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/find.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
#define VARIANT(args_...) ::boost::variant<args_>
#define DO_CAST(from_, to_, elem_)              \
    variant_cast<VARIANT to_> (VARIANT from_ {elem_})

    //! Require that a cast from from_ {elem_{}} to to_ results in elem_
#define REQUIRE_ID(from_, to_, elem_)                                          \
    do                                                                         \
    {                                                                          \
      elem_ e;                                                                 \
      using namespace ::boost::mpl;                                            \
      auto&& vv = DO_CAST (from_, to_, e);                                     \
      using variant_t = typename std::decay<decltype (vv)>::type;              \
      using it = typename find<variant_t::types, elem_>::type;                 \
      using pos = typename distance<begin<variant_t::types>::type, it>::type;  \
      BOOST_CHECK_MESSAGE ( vv.which() == pos{}                                \
                          , "expected " << typeid (elem_).name()               \
                          << ", got " << vv.type().name()                      \
                          );                                                   \
      BOOST_CHECK_EQUAL (::boost::get<elem_> (vv), e);                         \
    }                                                                          \
    while (false)

    //! Require that a cast from from_ {elem_{}} to to_ throws
#define REQUIRE_BAD_CAST(from_, to_, elem_)                                    \
    do                                                                         \
    {                                                                          \
      fhg::util::testing::require_exception                                    \
        ( [] { DO_CAST (from_, to_, elem_{}); }                                \
        , bad_variant_cast::make<VARIANT from_, VARIANT to_, elem_>()          \
        );                                                                     \
    }                                                                          \
    while (false)

      template<typename T>
        struct base
      {
        T _;
        base() : _ (testing::random<decltype (_)>{}()) {}
        bool operator== (base<T> const& other) const { return _ == other._; }
      };
      template<typename T>
        std::ostream& operator<< (std::ostream& os, base<T> const& x)
      {
        return os << x._;
      }

      using A = base<unsigned int>;
      using B = base<std::string>;
      using C = base<long>;

    BOOST_AUTO_TEST_CASE (to_same_types)
    {
      REQUIRE_ID ((A, B, C), (A, B, C), A);
      REQUIRE_ID ((A, B, C), (A, B, C), B);
      REQUIRE_ID ((A, B, C), (A, B, C), C);

      REQUIRE_ID ((A, B, C), (A, C, B), A);
      REQUIRE_ID ((A, B, C), (A, C, B), B);
      REQUIRE_ID ((A, B, C), (A, C, B), C);

      REQUIRE_ID ((A, B, C), (B, A, C), A);
      REQUIRE_ID ((A, B, C), (B, A, C), B);
      REQUIRE_ID ((A, B, C), (B, A, C), C);

      REQUIRE_ID ((A, B, C), (B, C, A), A);
      REQUIRE_ID ((A, B, C), (B, C, A), B);
      REQUIRE_ID ((A, B, C), (B, C, A), C);

      REQUIRE_ID ((A, B, C), (C, A, B), A);
      REQUIRE_ID ((A, B, C), (C, A, B), B);
      REQUIRE_ID ((A, B, C), (C, A, B), C);

      REQUIRE_ID ((A, B, C), (C, B, A), A);
      REQUIRE_ID ((A, B, C), (C, B, A), B);
      REQUIRE_ID ((A, B, C), (C, B, A), C);
    }

    BOOST_AUTO_TEST_CASE (to_less_types)
    {
      REQUIRE_ID ((A, B, C), (A, C), A);
      REQUIRE_BAD_CAST ((A, B, C), (A, C), B);
      REQUIRE_ID ((A, B, C), (A, C), C);

      REQUIRE_BAD_CAST ((A, B, C), (C), A);
      REQUIRE_BAD_CAST ((A, B, C), (C), B);
      REQUIRE_ID ((A, B, C), (C), C);
    }

    BOOST_AUTO_TEST_CASE (to_more_types)
    {
      REQUIRE_ID ((A), (C, A, B), A);
      REQUIRE_ID ((B), (C, A, B), B);
      REQUIRE_ID ((C), (C, A, B), C);

      REQUIRE_ID ((A, B), (C, A, B), A);
      REQUIRE_ID ((A, B), (C, A, B), B);
    }

    BOOST_AUTO_TEST_CASE (overlapping_types)
    {
      REQUIRE_BAD_CAST ((A, B), (B, C), A);
      REQUIRE_ID ((A, B), (B, C), B);
      REQUIRE_ID ((A, B), (B, C), B);

      REQUIRE_BAD_CAST ((A, C), (B, A), C);
      REQUIRE_ID ((A, C), (B, A), A);
    }

    BOOST_AUTO_TEST_CASE (completely_distinct_types)
    {
      REQUIRE_BAD_CAST ((A, B), (C), A);
      REQUIRE_BAD_CAST ((A, B), (C), B);
      REQUIRE_BAD_CAST ((C), (A, B), C);
    }

    BOOST_AUTO_TEST_CASE (variants_cast_casts_all_elements)
    {
      std::vector<::boost::variant<A, B>> from {A{}, B{}, B{}, A{}};

      auto&& to (variants_cast<::boost::variant<A, B, C>> (from));

      BOOST_REQUIRE_EQUAL (::boost::get<A> (from[0]), ::boost::get<A> (to[0]));
      BOOST_REQUIRE_EQUAL (::boost::get<B> (from[1]), ::boost::get<B> (to[1]));
      BOOST_REQUIRE_EQUAL (::boost::get<B> (from[2]), ::boost::get<B> (to[2]));
      BOOST_REQUIRE_EQUAL (::boost::get<A> (from[3]), ::boost::get<A> (to[3]));
    }
  }
}
