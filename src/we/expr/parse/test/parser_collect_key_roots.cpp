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

        return 1 + random_number() % 100;
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
      node::KeyRoots key_roots;

      parser ("").collect_key_roots (key_roots);

      BOOST_REQUIRE (key_roots.empty());
    }

    BOOST_AUTO_TEST_CASE (expression_without_reference_has_no_key_roots)
    {
      node::KeyRoots key_roots;

      parser ("1 + 2").collect_key_roots (key_roots);

      BOOST_REQUIRE (key_roots.empty());
    }

    BOOST_AUTO_TEST_CASE
      (expression_sequence_without_references_has_no_key_roots)
    {
      node::KeyRoots key_roots;

      parser ("1 + 2; 3 + 4").collect_key_roots (key_roots);

      BOOST_REQUIRE (key_roots.empty());
    }

    BOOST_AUTO_TEST_CASE (expression_with_reference_has_key_roots)
    {
      node::KeyRoots key_roots;

      auto const x (random_nonempty_path());

      parser (reference (x)).collect_key_roots (key_roots);

      node::KeyRoots const expected {x.front()};

      BOOST_TEST (key_roots == expected);
    }

    BOOST_AUTO_TEST_CASE (expression_sequence_with_references_has_key_roots)
    {
      node::KeyRoots key_roots;

      auto const N (random_small_positive_number());
      std::list<Path> xs (N);
      std::generate (xs.begin(), xs.end(), random_nonempty_path);

      parser ( fhg::util::print_container
               ( "", ";" , "", xs
               , [] (std::ostream& os, Path const& path) -> decltype (os)
                 {
                   return os << reference (path);
                 }
               ).string()
             ).collect_key_roots (key_roots);

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
