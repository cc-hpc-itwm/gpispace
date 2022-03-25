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

#include <util-generic/callable_signature.hpp>

#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    //! \note not in anonymous namespace to avoid clang not emitting
    //! it (-Wunneeded-member-function)
    struct fun
    {
      int operator() (fun const&) const;
      void member_function (std::string&&);
      static void static_function (std::string&&);
    };

    BOOST_AUTO_TEST_CASE (normal_signatures)
    {
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void()>, void());
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void (void)>, void());
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<void (int, float)>, void (int, float));

      BOOST_REQUIRE ((is_callable<void(), void()>{}));
      BOOST_REQUIRE ((is_callable<void (void), void()>{}));
      BOOST_REQUIRE ((is_callable<void (int, float), void (int, float)>{}));
    }

    extern int some_function (char const*);

    BOOST_AUTO_TEST_CASE (function_pointer)
    {
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<decltype (&some_function)>, int (char const*));
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<int (*) (char const*)>, int (char const*));

      BOOST_REQUIRE
        ((is_callable<decltype (&some_function), int (char const*)>{}));
      BOOST_REQUIRE ((is_callable<int (char const*), int (char const*)>{}));
    }

    BOOST_AUTO_TEST_CASE (functor)
    {
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (callable_signature<fun>, int (fun const&));

      BOOST_REQUIRE ((is_callable<fun, int (fun const&)>{}));
    }

    BOOST_AUTO_TEST_CASE (member_functions)
    {
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        ( callable_signature<decltype (&fun::member_function)>
        , callable_signature<decltype (&fun::static_function)>
        );
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        ( callable_signature<decltype (&fun::member_function)>
        , void (std::string&&)
        );

      BOOST_REQUIRE
        ((is_callable<decltype (&fun::static_function), void (std::string&&)>{}));
    }

    BOOST_AUTO_TEST_CASE (check_return_type)
    {
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (return_type<fun>, int);
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&fun::member_function)>, void);
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&fun::static_function)>, void);
      FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
        (return_type<decltype (&some_function)>, int);
    }
  }
}
