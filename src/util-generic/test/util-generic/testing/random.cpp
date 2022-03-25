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

#include <util-generic/fallthrough.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hard_integral_typedef.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/require_type.hpp>

#include <cmath>
#include <cstddef>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace
      {
        template<typename, typename = void>
          struct random_exists_and_is_callable : std::false_type {};

        template<typename T>
          struct random_exists_and_is_callable
            <T, cxx17::void_t<decltype (std::declval<T&>() = random<T>{}())>>
              : std::true_type {};

        struct type {};
        struct forward_declared_type;
      }

      BOOST_AUTO_TEST_CASE (random_not_existing_for_all_types)
      {
        BOOST_CHECK (!random_exists_and_is_callable<void>::value);
        BOOST_CHECK (!random_exists_and_is_callable<type>::value);
        BOOST_CHECK (!random_exists_and_is_callable<forward_declared_type>::value);
      }

      BOOST_AUTO_TEST_CASE (random_existing_for_integral_types)
      {
        BOOST_CHECK (random_exists_and_is_callable<bool>::value);
        BOOST_CHECK (random_exists_and_is_callable<char16_t>::value);
        BOOST_CHECK (random_exists_and_is_callable<char32_t>::value);
        BOOST_CHECK (random_exists_and_is_callable<char>::value);
        BOOST_CHECK (random_exists_and_is_callable<int>::value);
        BOOST_CHECK (random_exists_and_is_callable<long long>::value);
        BOOST_CHECK (random_exists_and_is_callable<long>::value);
        BOOST_CHECK (random_exists_and_is_callable<short>::value);
        BOOST_CHECK (random_exists_and_is_callable<signed char>::value);
        BOOST_CHECK (random_exists_and_is_callable<unsigned char>::value);
        BOOST_CHECK (random_exists_and_is_callable<unsigned int>::value);
        BOOST_CHECK (random_exists_and_is_callable<unsigned long long>::value);
        BOOST_CHECK (random_exists_and_is_callable<unsigned long>::value);
        BOOST_CHECK (random_exists_and_is_callable<unsigned short>::value);
        BOOST_CHECK (random_exists_and_is_callable<wchar_t>::value);
      }

      FHG_UTIL_HARD_INTEGRAL_TYPEDEF (test_t, std::size_t);
      BOOST_AUTO_TEST_CASE (random_existing_for_hard_integral_typedef)
      {
        BOOST_CHECK (random_exists_and_is_callable<test_t>::value);
      }

      BOOST_AUTO_TEST_CASE (random_existing_for_floating_point_types)
      {
        BOOST_CHECK (random_exists_and_is_callable<float>::value);
        BOOST_CHECK (random_exists_and_is_callable<double>::value);
        BOOST_CHECK (random_exists_and_is_callable<long double>::value);
      }

      BOOST_AUTO_TEST_CASE (random_exists_for_string)
      {
        BOOST_CHECK (random_exists_and_is_callable<std::string>::value);
      }

      namespace
      {
        template<typename, typename = void>
          struct randoms_exists_and_is_callable : std::false_type {};

        template<typename T>
          struct randoms_exists_and_is_callable
            < T
            , cxx17::void_t
                < decltype ( std::declval<T&>() = randoms<T>
                               (std::declval<std::size_t>())
                           )
                >
            > : std::true_type {};
      }

      BOOST_AUTO_TEST_CASE (randoms_defaults_to_random_as_generator)
      {
        FHG_UTIL_TESTING_REQUIRE_TYPE_EQUAL
          ( decltype (randoms<std::vector<int>>)
          , decltype (randoms<std::vector<int>, random<int>>)
          );
      }

      BOOST_AUTO_TEST_CASE (randoms_exists_for_sequence_containers)
      {
        BOOST_CHECK (randoms_exists_and_is_callable<std::basic_string<int>>::value);
        BOOST_CHECK (randoms_exists_and_is_callable<std::deque<int>>::value);
        BOOST_CHECK (randoms_exists_and_is_callable<std::forward_list<int>>::value);
        BOOST_CHECK (randoms_exists_and_is_callable<std::list<int>>::value);
        BOOST_CHECK (randoms_exists_and_is_callable<std::vector<int>>::value);
      }

      namespace
      {
        template<typename Generator, typename = void>
          struct randoms_with_custom_generator
            : std::false_type {};

        template<typename Generator>
          struct randoms_with_custom_generator
            < Generator
            , cxx17::void_t
                < decltype ( std::declval<std::vector<int>&>()
                           = randoms<std::vector<int>, Generator>
                               (std::declval<std::size_t>())
                           )
                >
            > : std::true_type {};

        struct custom_generator
        {
        };
      }

      BOOST_AUTO_TEST_CASE (randoms_exists_with_custom_generator)
      {
        BOOST_CHECK (randoms_with_custom_generator<custom_generator>::value);
        BOOST_CHECK (randoms_with_custom_generator<unique_random<int>>::value);
        BOOST_CHECK (randoms_with_custom_generator<random<int>>::value);
      }

      namespace
      {
        struct not_so_random_value_type
        {
          std::size_t value;
          not_so_random_value_type (std::size_t v) : value (v) {}
          bool operator== (not_so_random_value_type const& other) const
          {
            return value == other.value;
          }
        };

        not_so_random_value_type const first_result = random<std::size_t>{}();
        not_so_random_value_type const second_result = random<std::size_t>{}();
      }
    }
  }
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  (fhg::util::testing::not_so_random_value_type, v, v.value)
FHG_BOOST_TEST_LOG_VALUE_PRINTER
  (fhg::util::testing::not_so_random_value_type, os, v)
{
  os << v.value;
}

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<>
          struct random_impl<not_so_random_value_type>
        {
          std::size_t invocation_counter = 0;

          ~random_impl()
          {
            BOOST_REQUIRE_EQUAL (invocation_counter, 3);
          }

          not_so_random_value_type operator()()
          {
            FHG_UTIL_FINALLY ([&] { ++invocation_counter; });

            switch (invocation_counter)
            {
              case 0:
                FHG_UTIL_FALLTHROUGH;
              case 1:
                return first_result;
              case 2:
                return second_result;
              default:
                throw std::logic_error ("invoked mode more than three times");
            }
          }
        };
      }

      BOOST_AUTO_TEST_CASE (unique_random_calls_generator_until_it_has_gotten_a_unique_result)
      {
        unique_random<not_so_random_value_type> ur;
        BOOST_REQUIRE_EQUAL (ur(), first_result);
        BOOST_REQUIRE_EQUAL (ur(), second_result);
      }

      BOOST_AUTO_TEST_CASE (unique_random_calls_overwritten_generator)
      {
        struct pies_please
        {
          std::size_t i = 1;
          double operator()()
          {
            return static_cast<double> (i++) * M_PI;
                // ^ cast may loose precision
          }
        };

        auto const mmmmh (std::numeric_limits<double>::round_error());
        unique_random<double, pies_please> pies;
        BOOST_REQUIRE_CLOSE (pies(), 1 * M_PI, mmmmh);
        BOOST_REQUIRE_CLOSE (pies(), 2 * M_PI, mmmmh);
        BOOST_REQUIRE_CLOSE (pies(), 3 * M_PI, mmmmh);
      }

      BOOST_AUTO_TEST_CASE ( unique_random_uniqueness_is_per_object
                           , *::boost::unit_test::timeout (30)
                           )
      {
        struct constant_value_generator
        {
          int operator()() const
          {
            return 0;
          }
        };

        using unique_constant_value
          = unique_random<int, constant_value_generator>;
        BOOST_REQUIRE_EQUAL (unique_constant_value{}(), 0);
        // Test failure would trigger timeout, as unique_random busy
        // stalls when out of randomness and this second call could
        // never "roll" a different value but 0.
        BOOST_REQUIRE_EQUAL (unique_constant_value{}(), 0);
      }

      BOOST_AUTO_TEST_CASE (randoms_generates_requested_amount_of_values)
      {
        std::size_t const requested (random<std::size_t>{} (2048, 1024));
        auto const values (randoms<std::vector<std::size_t>> (requested));
        BOOST_REQUIRE_EQUAL (values.size(), requested);
      }

      BOOST_AUTO_TEST_CASE
        (randoms_with_unique_generates_requested_amount_of_unique_values)
      {
        std::size_t const requested (random<std::size_t>{} (2048, 1024));
        using T = std::size_t;
        auto const values
          (randoms<std::vector<T>, unique_random<T>> (requested));
        std::set<T> const uniqued (values.begin(), values.end());
        BOOST_REQUIRE_EQUAL (uniqued.size(), requested);
      }

      BOOST_AUTO_TEST_CASE
        (unique_randoms_generates_requested_amount_of_unique_values)
      {
        std::size_t const requested (random<std::size_t>{} (2048, 1024));
        using T = float;
        auto const values (unique_randoms<std::vector<T>> (requested));
        std::set<T> const uniqued (values.begin(), values.end());
        BOOST_REQUIRE_EQUAL (uniqued.size(), requested);
      }

      BOOST_AUTO_TEST_CASE
        (tenthousand_floats_are_zero_or_normal_and_have_fair_sign_distribution)
      {
        constexpr std::size_t const rolls {10000};

        std::size_t negative {0};
        std::size_t positive {0};

        for (std::size_t remaining {rolls}; remaining != 0; --remaining)
        {
          float const rolled (random<float>{}());

          negative += rolled < 0.0f;
          positive += rolled > 0.0f;

          BOOST_REQUIRE
            (std::isnormal (rolled) || std::equal_to<float>{} (rolled, 0.0f));
        }

        BOOST_REQUIRE_LE (negative, 9.0 / 16.0 * rolls);
        BOOST_REQUIRE_LE (positive, 9.0 / 16.0 * rolls);
      }
    }
  }
}
