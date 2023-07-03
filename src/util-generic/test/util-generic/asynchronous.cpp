// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/asynchronous.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/range/adaptor/uniqued.hpp>

#include <algorithm>
#include <atomic>
#include <numeric>
#include <type_traits>
#include <vector>

BOOST_AUTO_TEST_CASE (all_functions_are_executed)
{
  std::vector<unsigned long> const input
    { fhg::util::testing::randoms<std::remove_const<decltype (input)>::type>
        (fhg::util::testing::random<decltype (input)::size_type>()() % 1000)
    };

  std::atomic<decltype (input)::value_type> sum (0);

  fhg::util::asynchronous
    (input, [&sum] (decltype (input)::value_type const& x) { sum += x; });

  BOOST_REQUIRE_EQUAL
    ( sum.load()
    , std::accumulate ( input.begin(), input.end()
                      , static_cast<decltype (input)::value_type> (0)
                      )
    );
}

BOOST_AUTO_TEST_CASE (exceptions_are_transported)
{
  using T = int;

  T const number {fhg::util::testing::random<T>()()};

  fhg::util::testing::require_exception
    ( [number]()
      {
        fhg::util::asynchronous
          ( std::list<T> {number}
          , [] (T const& x)
            {
              throw std::runtime_error (std::to_string (x));
            }
          );
      }
    , std::runtime_error (std::to_string (number))
    );

  fhg::util::testing::require_exception
    ( [number]()
      {
        //! \note two times the same number to avoid ordering dependencies
        fhg::util::asynchronous
          ( std::list<T> {number, number}
          , [] (T const& x)
            {
              throw std::runtime_error (std::to_string (x));
            }
          );
      }
    , std::runtime_error
        (std::to_string (number) + '\n' + std::to_string (number))
    );
}

BOOST_AUTO_TEST_CASE (ranges)
{
  std::list<int> const list {0,1,2,2,3,4,4,5};

  std::atomic<int> sum (0);

  fhg::util::asynchronous
    ( list | ::boost::adaptors::uniqued
    , [&sum] (int const& x) { sum += x; }
    );

  BOOST_REQUIRE_EQUAL (sum.load(), 15);
}
