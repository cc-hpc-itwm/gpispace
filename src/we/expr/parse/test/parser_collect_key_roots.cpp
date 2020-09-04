// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/expr/parse/parser.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <algorithm>
#include <list>
#include <string>

namespace expr
{
  namespace parse
  {
    namespace
    {
      std::size_t random_small_positive_number()
      {
        static auto random_number (fhg::util::testing::random<std::size_t>{});

        return random_number(100, 1);
      }

      using Path = std::list<std::string>;

      Path random_nonempty_path()
      {
        using fhg::util::testing::random_identifier_without_leading_underscore;

        Path path (random_small_positive_number());

        std::generate ( path.begin()
                      , path.end()
                      , random_identifier_without_leading_underscore
                      );

        return path;
      }

      std::string reference (Path path)
      {
        return fhg::util::print_container ("${", ".", "}", path).string();
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
        ( parser ( fhg::util::print_container
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
}
