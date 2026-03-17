// Copyright (C) 2019-2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/expr/parse/parser.hpp>

#include <gspc/util/print_container.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/random/string.hpp>
#include <gspc/testing/require_exception.hpp>

#include <algorithm>
#include <list>
#include <string>


  namespace gspc::we::expr::parse
  {
    namespace
    {
      std::size_t random_small_positive_number()
      {
        static auto random_number (gspc::testing::random<std::size_t>{});

        return random_number(100, 1);
      }

      using Path = std::list<std::string>;

      Path random_nonempty_path()
      {
        using gspc::testing::random_identifier_without_leading_underscore;

        Path path (random_small_positive_number());

        std::generate ( path.begin()
                      , path.end()
                      , random_identifier_without_leading_underscore
                      );

        return path;
      }

      std::string reference (Path path)
      {
        return gspc::util::print_container ("${", ".", "}", path).string();
      }
    }

    BOOST_AUTO_TEST_CASE (empty_expression_has_no_key_roots)
    {
      BOOST_REQUIRE (parser ("").key_roots().empty());
    }

    BOOST_AUTO_TEST_CASE (expression_without_reference_has_no_key_roots)
    {
      BOOST_REQUIRE (parser ("1 + 2").key_roots().empty());
    }

    BOOST_AUTO_TEST_CASE
      (expression_sequence_without_references_has_no_key_roots)
    {
      BOOST_REQUIRE (parser ("1 + 2; 3 + 4").key_roots().empty());
    }

    BOOST_AUTO_TEST_CASE (expression_with_reference_has_key_roots)
    {
      auto const x (random_nonempty_path());

      node::KeyRoots const expected {x.front()};

      BOOST_TEST (parser (reference (x)).key_roots () == expected);
    }

    BOOST_AUTO_TEST_CASE (expression_sequence_with_references_has_key_roots)
    {
      auto const N (random_small_positive_number());
      std::list<Path> xs (N);
      std::generate (xs.begin(), xs.end(), random_nonempty_path);

      auto const key_roots
        ( parser ( gspc::util::print_container
                   ( "", ";" , "", xs
                   , [] (std::ostream& os, Path const& path) -> decltype (os)
                     {
                       return os << reference (path);
                     }
                   ).string()
                 ).key_roots()
        );

      node::KeyRoots expected;

      std::for_each ( xs.cbegin()
                    , xs.cend()
                    , [&] (Path const& path)
                      {
                        expected.emplace (path.front());
                      }
                    );

      BOOST_TEST (key_roots == expected);
    }
  }
