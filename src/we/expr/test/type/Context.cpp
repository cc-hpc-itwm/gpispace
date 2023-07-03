// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/type/Context.hpp>

#include <we/expr/exception.hpp>
#include <we/expr/type/Type.hpp>
#include <we/expr/type/testing/types.hpp>

#include <util-generic/join.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <iterator>
#include <string>
#include <vector>

namespace expr
{
  namespace type
  {
    namespace
    {
      template<typename IT>
        Type chain (IT begin, IT end, Type const& type)
      {
        if (begin == end)
        {
          return type;
        }

        return Struct ({{*begin, chain (std::next (begin), end, type)}});
      }

      struct RandomIdentifier
      {
        std::string operator()() const
        {
          return _random_string (decltype (_random_string)::identifier{}, 50);
        }

        fhg::util::testing::random<std::string> _random_string;
      };

      struct RandomPath
      {
        Path::Particles operator() (std::size_t min = 0) const
        {
          return fhg::util::testing::randoms<Path::Particles>
            ( _random_count (20, min)
            , _random_identifier
            );
        }

        fhg::util::testing::random<std::size_t> _random_count;
        RandomIdentifier _random_identifier;
      };
    }

    BOOST_AUTO_TEST_CASE (empty_context_has_empty_struct_at_empty_path)
    {
      Context const context;
      Path const path;

      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).get(), Type {Struct {{}}});
    }

    BOOST_DATA_TEST_CASE
      ( at_nonempty_path_of_bound_type_in_empty_context_is_identity
      , testing::all_types()
      , td
      )
    {
      Context context;
      Path const path (RandomPath{} (1));
      auto const type (td.type);

      BOOST_REQUIRE_EQUAL (type, context.bind (path, type));
      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).get(), type);
    }

    BOOST_DATA_TEST_CASE
      ( empty_context_can_bind_to_anything_at_any_nonempty_path
      , testing::all_types()
      , td
      )
    {
      Context context;
      Path const path (RandomPath{} (1));
      auto const type (td.type);

      BOOST_REQUIRE_EQUAL (type, context.bind (path, type));
      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).get(), type);
    }

    BOOST_DATA_TEST_CASE
      ( at_nonempty_path_can_retrieve_whole_chains
      , testing::all_types()
      , td
      )
    {
      Context context;
      Path const path (RandomPath{} (1));
      auto const type (td.type);

      BOOST_REQUIRE_EQUAL (type, context.bind (path, type));
      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).get(), type);

      for ( auto begin (std::begin (path)), key (begin), end (std::end (path))
          ; key != end
          ; ++key
          )
      {
        Path const prefix (begin, key);

        BOOST_REQUIRE (!!context.at (prefix));

        BOOST_REQUIRE_EQUAL (context.at (prefix).get(), chain (key, end, type));
      }
    }

    BOOST_DATA_TEST_CASE
      ( struct_fields_can_be_bound_individually
      , testing::all_types()
      * testing::all_types()
      , tdA
      , tdB
      )
    {
      using fhg::util::testing::unique_random;

      Context context;
      RandomPath const random_path;
      auto const common_prefix (random_path());
      unique_random<std::string, RandomIdentifier> distinct_particle;
      auto const particleA (distinct_particle());
      auto const particleB (distinct_particle());
      auto const suffixA (random_path());
      auto const suffixB (random_path());

      auto pathA (common_prefix);
      pathA.emplace_back (particleA);
      pathA.insert (std::end (pathA), std::begin (suffixA), std::end (suffixA));

      auto pathB (common_prefix);
      pathB.emplace_back (particleB);
      pathB.insert (std::end (pathB), std::begin (suffixB), std::end (suffixB));

      auto const typeA (tdA.type);
      auto const typeB (tdB.type);

      BOOST_REQUIRE_EQUAL (typeA, context.bind (pathA, typeA));
      BOOST_REQUIRE (!!context.at (pathA));
      BOOST_REQUIRE_EQUAL (context.at (pathA).get(), typeA);

      BOOST_REQUIRE_EQUAL (typeB, context.bind (pathB, typeB));
      BOOST_REQUIRE (!!context.at (pathB));
      BOOST_REQUIRE_EQUAL (context.at (pathB).get(), typeB);

      Struct const sub
        ( { {particleA, chain (std::begin (suffixA), std::end (suffixA), typeA)}
          , {particleB, chain (std::begin (suffixB), std::end (suffixB), typeB)}
          }
        );

      BOOST_REQUIRE (!!context.at (common_prefix));
      BOOST_REQUIRE_EQUAL (context.at (common_prefix).get(), Type {sub});
    }

    BOOST_DATA_TEST_CASE
      ( bind_at_leaf_tries_to_overwrite
      , testing::all_types()
      * testing::all_types()
      , tdA
      , tdB
      )
    {
      Context context;
      Path const path (RandomPath{} (1));
      auto const typeA (tdA.type);
      auto const typeB (tdB.type);

      BOOST_REQUIRE_EQUAL (typeA, context.bind (path, typeA));
      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).get(), typeA);

      if (typeA != typeB)
      {
        fhg::util::testing::require_exception
          ( [&]
            {
              (void) context.bind (path, typeB);
            }
          , fhg::util::testing::make_nested
            ( exception::type::error
              ( ::boost::format ("expr::type::Context::bind (%1%, '%2%')")
              % path
              % typeB
              )
            , exception::type::error
              ( ::boost::format ("At %3%: Can not assign a value of type '%2%' to a value of type '%1%'")
              % typeA
              % typeB
              % path
              )
            )
          );
      }
    }

    BOOST_DATA_TEST_CASE
      ( bind_at_non_leaves_tries_to_overwrite
      , testing::all_types() * testing::all_types_except (Struct ({}))
      , tdA
      , tdB
      )
    {
      RandomPath const random_path;
      auto const prefix (random_path (1));
      auto const suffix (random_path (1));
      auto path (prefix);
      path.insert (std::end (path), std::begin (suffix), std::end (suffix));

      Context context;

      auto const typeA (tdA.type);
      auto const typeB (tdB.type);

      context.bind (path, typeA);

      fhg::util::testing::require_exception
        ( [&]
          {
            (void) context.bind (prefix, typeB);
          }
        , fhg::util::testing::make_nested
          ( exception::type::error
            ( ::boost::format ("expr::type::Context::bind (%1%, '%2%')")
            % Path {prefix}
            % typeB
            )
          , exception::type::error
            ( ::boost::format ("At %3%: Can not assign a value of type '%2%' to a value of type '%1%'")
            % chain (std::begin (suffix), std::end (suffix), typeA)
            % typeB
            % Path {prefix}
            )
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( bind_at_nonempty_path_below_a_non_struct_leaf_throws
      , testing::all_types_except (Struct ({})) * testing::all_types()
      , tdLeaf
      , tdDeeper
      )
    {
      Context context;
      RandomPath const random_path;
      auto const prefix (random_path (1));
      auto const suffix (random_path (1));
      auto path (prefix);
      path.insert (std::end (path), std::begin (suffix), std::end (suffix));

      auto const leaf (tdLeaf.type);
      auto const deeper (tdDeeper.type);

      (void) context.bind (prefix, leaf);

      fhg::util::testing::require_exception
        ( [&]
          {
            (void) context.bind (path, deeper);
          }
        , fhg::util::testing::make_nested
          ( exception::type::error
            ( ::boost::format ("expr::type::Context::bind (${%1%}, '%2%')")
            % fhg::util::join (path, ".")
            % deeper
            )
          , exception::type::error
            ( ::boost::format ("Not a struct at ${%1%}: '%2%'")
            % fhg::util::join (prefix, ".")
            % leaf
            )
          )
        );
    }

    BOOST_DATA_TEST_CASE
      ( at_nonempty_path_below_a_leaf_yields_nothing
      , testing::all_types()
      , tdLeaf
      )
    {
      Context context;
      RandomPath const random_path;
      auto const prefix (random_path (1));
      auto const suffix (random_path (1));
      auto path (prefix);
      path.insert (std::end (path), std::begin (suffix), std::end (suffix));

      auto const leaf (tdLeaf.type);

      (void) context.bind (prefix, leaf);

      BOOST_REQUIRE (!context.at (path));
    }
  }
}
