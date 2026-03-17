// Copyright (C) 2021-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/type/Context.hpp>

#include <gspc/we/expr/exception.hpp>
#include <gspc/we/expr/type/Type.hpp>
#include <gspc/we/expr/type/testing/types.hpp>

#include <gspc/util/join.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/util/join.formatter.hpp>
#include <gspc/we/expr/type/Path.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <fmt/core.h>
#include <iterator>
#include <string>
#include <vector>


  namespace gspc::we::expr::type
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

        gspc::testing::random<std::string> _random_string;
      };

      struct RandomPath
      {
        Path::Particles operator() (std::size_t min = 0) const
        {
          return gspc::testing::randoms<Path::Particles>
            ( _random_count (20, min)
            , _random_identifier
            );
        }

        gspc::testing::random<std::size_t> _random_count;
        RandomIdentifier _random_identifier;
      };
    }

    BOOST_AUTO_TEST_CASE (empty_context_has_empty_struct_at_empty_path)
    {
      Context const context;
      Path const path;

      BOOST_REQUIRE (!!context.at (path));
      BOOST_REQUIRE_EQUAL (context.at (path).value(), Type {Struct {{}}});
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
      BOOST_REQUIRE_EQUAL (context.at (path).value(), type);
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
      BOOST_REQUIRE_EQUAL (context.at (path).value(), type);
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
      BOOST_REQUIRE_EQUAL (context.at (path).value(), type);

      for ( auto begin (std::begin (path)), key (begin), end (std::end (path))
          ; key != end
          ; ++key
          )
      {
        Path const prefix (begin, key);

        BOOST_REQUIRE (!!context.at (prefix));

        BOOST_REQUIRE_EQUAL (context.at (prefix).value(), chain (key, end, type));
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
      using gspc::testing::unique_random;

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
      BOOST_REQUIRE_EQUAL (context.at (pathA).value(), typeA);

      BOOST_REQUIRE_EQUAL (typeB, context.bind (pathB, typeB));
      BOOST_REQUIRE (!!context.at (pathB));
      BOOST_REQUIRE_EQUAL (context.at (pathB).value(), typeB);

      Struct const sub
        ( { {particleA, chain (std::begin (suffixA), std::end (suffixA), typeA)}
          , {particleB, chain (std::begin (suffixB), std::end (suffixB), typeB)}
          }
        );

      BOOST_REQUIRE (!!context.at (common_prefix));
      BOOST_REQUIRE_EQUAL (context.at (common_prefix).value(), Type {sub});
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
      BOOST_REQUIRE_EQUAL (context.at (path).value(), typeA);

      if (typeA != typeB)
      {
        gspc::testing::require_exception
          ( [&]
            {
              (void) context.bind (path, typeB);
            }
          , gspc::testing::make_nested
            ( exception::type::error
              { fmt::format ( "gspc::we::expr::type::Context::bind ({}, '{}')"
                            , path
                            , typeB
                            )
              }
            , exception::type::error
              { fmt::format
                  ( "At {2}: Can not assign a value of type '{1}' to a value of type '{0}'"
                  , typeA
                  , typeB
                  , path
                  )
              }
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

      gspc::testing::require_exception
        ( [&]
          {
            (void) context.bind (prefix, typeB);
          }
        , gspc::testing::make_nested
          ( exception::type::error
            { fmt::format ( "gspc::we::expr::type::Context::bind ({}, '{}')"
                          , Path {prefix}
                          , typeB
                          )
            }
          , exception::type::error
            { fmt::format ( "At {2}: Can not assign a value of type '{1}' to a value of type '{0}'"
                          , chain (std::begin (suffix), std::end (suffix), typeA)
                          , typeB
                          , Path {prefix}
                          )
            }
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

      gspc::testing::require_exception
        ( [&]
          {
            (void) context.bind (path, deeper);
          }
        , gspc::testing::make_nested
          ( exception::type::error
            { fmt::format ( "gspc::we::expr::type::Context::bind (${{{}}}, '{}')"
                          , gspc::util::join (path, ".")
                          , deeper
                          )
            }
          , exception::type::error
            { fmt::format ( "Not a struct at ${{{}}}: '{}'"
                          , gspc::util::join (prefix, ".")
                          , leaf
                          )
            }
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
