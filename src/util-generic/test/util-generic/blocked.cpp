// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/blocked.hpp>
#include <util-generic/divru.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/warning.hpp>

#include <numeric>
#include <string>

namespace
{
  template<typename T>
    std::function<T (T const&)> identity()
  {
    return [] (T const& x) { return x; };
  }
}

BOOST_AUTO_TEST_CASE (no_block_size_means_unblocked)
{
  std::size_t const number_of_elements
    {fhg::util::testing::random<std::size_t>{} (1000, 1)};

  std::list<int> container (number_of_elements);
  std::iota (container.begin(), container.end(), 0);

  using iterator = std::list<int>::const_iterator;
  std::vector<std::pair<iterator, iterator>> called_for;

  fhg::util::blocked
    ( container.begin()
    , container.end()
    , ::boost::none
    , [&] (iterator const& begin, iterator const& end)
      {
        called_for.emplace_back (begin, end);
      }
    );

  BOOST_REQUIRE_EQUAL (called_for.size(), 1u);
  BOOST_REQUIRE (called_for[0].first == container.begin());
  BOOST_REQUIRE (called_for[0].second == container.end());
}

BOOST_AUTO_TEST_CASE (blocked)
{
  std::size_t const block_size
    (fhg::util::testing::random<std::size_t>{} (100, 1));
  std::size_t const number_of_elements
    {fhg::util::testing::random<std::size_t>{} (1000)};

  std::list<int> container (number_of_elements);
  std::iota (container.begin(), container.end(), 0);

  using iterator = std::list<int>::const_iterator;
  std::vector<std::pair<iterator, iterator>> called_for;

  fhg::util::blocked
    ( container.begin()
    , container.end()
    , block_size
    , [&] (iterator const& begin, iterator const& end)
      {
        called_for.emplace_back (begin, end);
      }
    );

  BOOST_REQUIRE_EQUAL
    (called_for.size(), fhg::util::divru (number_of_elements, block_size));

  std::size_t elem (0);

  for (auto const& call : called_for)
  {
    auto const begin (call.first);
    auto const end (call.second);

    std::size_t const count
      ( fhg::util::suppress_warning::sign_conversion<std::size_t>
          ( std::distance (begin, end)
          , "begin < end"
          )
      );

    BOOST_REQUIRE
      (count == block_size || elem + count == number_of_elements);

    std::list<int> expected (count);
    std::iota (expected.begin(), expected.end(), elem);

    BOOST_REQUIRE_EQUAL_COLLECTIONS
      (begin, end, expected.begin(), expected.end());

    elem += count;
  }

  BOOST_REQUIRE_EQUAL (elem, number_of_elements);
}

BOOST_AUTO_TEST_CASE (block_size_zero_throws_on_nonempty_list)
{
  fhg::util::testing::require_exception
    ( []()
      {
        std::list<int> const container
          (fhg::util::testing::random<std::size_t>{} (1000, 1));

        fhg::util::blocked
          ( container.begin()
          , container.end()
          , std::size_t (0)
          , [] ( std::list<int>::const_iterator const&
               , std::list<int>::const_iterator const&
               )
            {
              return;
            }
          );
      }
    , std::invalid_argument ("blocksize must be positive")
    );
}

BOOST_AUTO_TEST_CASE (block_size_zero_is_okay_for_empty_lists)
{
  std::list<int> const container;

  fhg::util::blocked
    ( container.begin()
    , container.end()
    , std::size_t (0)
    , [] ( std::list<int>::const_iterator const&
         , std::list<int>::const_iterator const&
         )
      {
        BOOST_FAIL ("unexpected call to function");
      }
    );
}

BOOST_AUTO_TEST_CASE (blocked_async)
{
  auto&& fun
    ([] (int const& key)
     {
       if (key % 2)
       {
         throw std::runtime_error ("odd: " + std::to_string (key));
       }
     }
    );

  std::size_t const block_size
    (fhg::util::testing::random<std::size_t>{} (100, 1));
  std::size_t const number_of_elements
    {fhg::util::testing::random<std::size_t>{} (1000)};

  std::list<int> container (number_of_elements);
  std::iota (container.begin(), container.end(), 0);

  std::pair< std::unordered_set<int>
           , std::unordered_map<int, std::exception_ptr>
           > const result
    ( fhg::util::blocked_async<int>
        ( container
        , block_size
        , identity<int>()
        , fun
        )
    );

  BOOST_REQUIRE_EQUAL
    (result.first.size() + result.second.size(), number_of_elements);

  for (int i : container)
  {
    if (i % 2)
    {
      BOOST_REQUIRE_EQUAL (result.first.count (i), 0);
      BOOST_REQUIRE_EQUAL (result.second.count (i), 1);
      fhg::util::testing::require_exception
        ( [&result, &i]()
          {
            std::rethrow_exception (result.second.at (i));
          }
        , std::runtime_error ("odd: " + std::to_string (i))
        );
    }
    else
    {
      BOOST_REQUIRE_EQUAL (result.first.count (i), 1);
      BOOST_REQUIRE_EQUAL (result.second.count (i), 0);
    }
  }
}

BOOST_AUTO_TEST_CASE (blocked_async_with_results)
{
  auto&& fun
    ([] (int const& key)
     {
       if (key % 2)
       {
         throw std::runtime_error ("odd: " + std::to_string (key));
       }

       return std::to_string (key);
     }
    );

  std::size_t const block_size
    (fhg::util::testing::random<std::size_t>{} (100, 1));
  std::size_t const number_of_elements
    {fhg::util::testing::random<std::size_t>{} (1000)};

  std::list<int> container (number_of_elements);
  std::iota (container.begin(), container.end(), 0);

  std::pair< std::unordered_map<int, std::string>
           , std::unordered_map<int, std::exception_ptr>
           > const result
    ( fhg::util::blocked_async_with_results<int, std::string>
        ( container
        , block_size
        , identity<int>()
        , fun
        )
    );

  BOOST_REQUIRE_EQUAL
    (result.first.size() + result.second.size(), number_of_elements);

  for (int i : container)
  {
    if (i % 2)
    {
      BOOST_REQUIRE_EQUAL (result.first.count (i), 0);
      BOOST_REQUIRE_EQUAL (result.second.count (i), 1);
      fhg::util::testing::require_exception
        ( [&result, &i]()
          {
            std::rethrow_exception (result.second.at (i));
          }
        , std::runtime_error ("odd: " + std::to_string (i))
        );
    }
    else
    {
      BOOST_REQUIRE_EQUAL (result.first.count (i), 1);
      BOOST_REQUIRE_EQUAL (result.second.count (i), 0);
      BOOST_REQUIRE_EQUAL (result.first.at (i), std::to_string (i));
    }
  }
}
