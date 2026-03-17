// Copyright (C) 2019-2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/we/expr/parse/node.hpp>
#include <gspc/we/expr/token/testing/all_tokens.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <algorithm>
#include <functional>



    namespace gspc::we::expr::parse::node
    {
      namespace
      {
        Key random_nonempty_key()
        {
          static auto random_number (gspc::testing::random<std::size_t>{});

          return gspc::testing::randoms<Key> (random_number(100, 1));
        }
      }

      BOOST_AUTO_TEST_CASE (constant_has_no_key_roots)
      {
        KeyRoots key_roots;

        auto const constant (gspc::pnet::type::value::value_type{});

        collect_key_roots (constant, key_roots);

        BOOST_REQUIRE (key_roots.empty());
      }
      BOOST_AUTO_TEST_CASE (empty_key_has_no_key_roots_and_throws)
      {
        KeyRoots key_roots;

        auto const empty_key (Key{});

        gspc::testing::require_exception
          ( [&]
            {
              collect_key_roots (empty_key, key_roots);
            }
          , std::invalid_argument ("collect_key_roots: empty key")
          );

        BOOST_REQUIRE (key_roots.empty());
      }
      BOOST_AUTO_TEST_CASE (nonempty_key_has_key_roots)
      {
        KeyRoots key_roots;

        Key const nonempty_key (random_nonempty_key());

        collect_key_roots (nonempty_key, key_roots);

        KeyRoots const expected {nonempty_key.front()};

        BOOST_TEST (key_roots == expected);
      }
      BOOST_DATA_TEST_CASE
        ( unary_is_traversed_for_key_roots
        , token::testing::all_tokens()
        , token
        )
      {
        KeyRoots key_roots;

        Key const nonempty_key (random_nonempty_key());

        collect_key_roots (unary_t (token, nonempty_key), key_roots);

        KeyRoots const expected {nonempty_key.front()};

        BOOST_TEST (key_roots == expected);
      }
      BOOST_DATA_TEST_CASE
        ( binary_is_traversed_for_key_roots
        , token::testing::all_tokens()
        , token
        )
      {
        KeyRoots key_roots;

        Key const nonempty_key1 (random_nonempty_key());
        Key const nonempty_key2 (random_nonempty_key());

        collect_key_roots
          (binary_t (token, nonempty_key1, nonempty_key2), key_roots);

        KeyRoots const expected {nonempty_key1.front(), nonempty_key2.front()};

        BOOST_TEST (key_roots == expected);
      }
      BOOST_DATA_TEST_CASE
        ( ternary_is_traversed_for_key_roots
        , token::testing::all_tokens()
        , token
        )
      {
        KeyRoots key_roots;

        Key const nonempty_key1 (random_nonempty_key());
        Key const nonempty_key2 (random_nonempty_key());
        Key const nonempty_key3 (random_nonempty_key());

        collect_key_roots
          ( ternary_t (token, nonempty_key1, nonempty_key2, nonempty_key3)
          , key_roots
          );

        KeyRoots const expected
          {nonempty_key1.front(), nonempty_key2.front(), nonempty_key3.front()};

        BOOST_TEST (key_roots == expected);
      }
    }
