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

#include <boost/test/unit_test.hpp>

#include <util-generic/cxx17/apply.hpp>
#include <util-generic/cxx17/void_t.hpp>
#include <util-generic/testing/printer/tuple.hpp>
#include <util-generic/testing/random.hpp>

#include <array>
#include <tuple>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      BOOST_AUTO_TEST_CASE (actually_calls_with_argument)
      {
        std::tuple<int> const tuple (testing::random<int>{}());
        std::size_t was_called (0);
        cxx17::apply ( [&] (int value)
                       {
                         ++was_called;
                         BOOST_REQUIRE_EQUAL (std::make_tuple (value), tuple);
                       }
                     , tuple
                     );
        BOOST_REQUIRE_EQUAL (was_called, 1);
      }

      BOOST_AUTO_TEST_CASE (actually_calls_with_arguments)
      {
        std::tuple<int, std::size_t> const tuple
          (testing::random<int>{}(), testing::random<std::size_t>{}());
        std::size_t was_called (0);
        cxx17::apply ( [&] (int v1, std::size_t v2)
                       {
                         ++was_called;
                         BOOST_REQUIRE_EQUAL (std::make_tuple (v1, v2), tuple);
                       }
                     , tuple
                     );
        BOOST_REQUIRE_EQUAL (was_called, 1);
      }

      BOOST_AUTO_TEST_CASE (transports_return_value)
      {
        std::size_t const rolled (testing::random<std::size_t>{}());

        BOOST_REQUIRE_EQUAL
          (rolled, cxx17::apply ([&] { return rolled; }, std::tuple<>{}));
      }

      namespace
      {
        template<typename, typename, typename = void>
          struct is_callable : std::false_type {};

        template<typename Op, typename Tuple>
          struct is_callable< Op
                            , Tuple
                            , void_t
                                <decltype (cxx17::apply ( std::declval<Op>()
                                                        , std::declval<Tuple>()
                                                        )
                                          )
                                >
                            >
            : std::true_type {};
      }

      struct ref_overloaded_operator
      {
        std::size_t ref_counter = 0;
        std::size_t refref_counter = 0;
        void operator()() & { ++ref_counter; }
        void operator()() && { ++refref_counter; }
      };

      BOOST_AUTO_TEST_CASE (refref_overloaded_operator_optimization_is_possible)
      {
        {
          ref_overloaded_operator fun;
          cxx17::apply (fun, std::tuple<>{});
          BOOST_REQUIRE_EQUAL (fun.ref_counter, 1);
          BOOST_REQUIRE_EQUAL (fun.refref_counter, 0);
        }
        {
          ref_overloaded_operator fun;
          //! \note move doesn't actually do anything but cast
          apply (std::move (fun), std::tuple<>{});
          BOOST_REQUIRE_EQUAL (fun.ref_counter, 0);
          BOOST_REQUIRE_EQUAL (fun.refref_counter, 1);
        }
      }

      BOOST_AUTO_TEST_CASE (works_not_only_with_std_tuple)
      {
        BOOST_REQUIRE
          ((is_callable<void (int, float), std::pair<int, float>>::value));
        BOOST_REQUIRE
          ((is_callable<void (int, int, int), std::array<int, 3>>::value));
      }
    }
  }
}
