// Copyright (C) 2021-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/type/AssignResult.hpp>

#include <gspc/we/expr/exception.hpp>
#include <gspc/we/expr/type/Type.hpp>
#include <gspc/we/expr/type/testing/types.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/we/expr/type/Path.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <iterator>
#include <string>
#include <vector>


  namespace gspc::we::expr::type
  {
    namespace
    {
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

      template<typename Container>
        Container random_selection (Container container)
      {
        gspc::testing::random<bool> random_bool;
        Container selection;

        std::copy_if ( std::begin (container), std::end (container)
                     , std::inserter (selection, std::end (selection))
                     , [&] (auto const&) { return random_bool(); }
                     );

        return selection;
      }

      Types random_type_collection()
      {
        Types types;

        for (auto const& type : random_selection (testing::all_types()))
        {
          types._types.emplace (type.type);
        }

        return types;
      }
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_with_the_same_type_return_rhs
      , testing::all_types()
      , td
      )
    {
      Path const path (RandomPath{}());

      BOOST_REQUIRE_EQUAL (td.type, assign_result (path, td.type, td.type));
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_with_different_type_throws
      , testing::all_types() * testing::all_types()
      , tdA
      , tdB
      )
    {
      if (tdA.type != tdB.type)
      {
        Path const path (RandomPath{}());

        gspc::testing::require_exception
          ( [&]
            {
              (void) assign_result (path, tdA.type, tdB.type);
            }
          , exception::type::error
            { fmt::format
               ( "At {2}: Can not assign a value of type '{1}'"
                 " to a value of type '{0}'"
               , tdA.type
               , tdB.type
               , path
               )
            }
          );
      }
    }

    BOOST_AUTO_TEST_CASE (assign_result_shared_same_cleanup_place_returns_rhs)
    {
      Path const path (RandomPath{}());
      Type const shared_A (Shared {"cleanup"});

      BOOST_REQUIRE_EQUAL (shared_A, assign_result (path, shared_A, shared_A));
    }

    BOOST_AUTO_TEST_CASE (assign_result_shared_different_cleanup_place_throws)
    {
      Path const path (RandomPath{}());
      Type const shared_A (Shared {"A"});
      Type const shared_B (Shared {"B"});

      gspc::testing::require_exception
        ( [&]
          {
            std::ignore = assign_result (path, shared_A, shared_B);
          }
        , exception::type::error
          { fmt::format
              ( "At {2}: Can not assign a value of type 'shared_{1}'"
                " to a value of type 'shared_{0}'"
              , "A"
              , "B"
              , path
              )
          }
        );
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_of_collections_at_lhs_returns_rhs
      , testing::all_types()
      , rhs
      )
    {
      Path const path (RandomPath{}());
      auto const lhs (random_type_collection());

      BOOST_REQUIRE_EQUAL (rhs.type, assign_result (path, lhs, rhs.type));
    }

    BOOST_AUTO_TEST_CASE (assign_result_for_lists_is_rhs)
    {
      Path const path (RandomPath{}());
      Type const lhs (List {random_type_collection()});
      Type const rhs (List {random_type_collection()});

      BOOST_REQUIRE_EQUAL (rhs, assign_result (path, lhs, rhs));
    }

    BOOST_AUTO_TEST_CASE (assign_result_for_set_is_rhs)
    {
      Path const path (RandomPath{}());
      Type const lhs (Set {random_type_collection()});
      Type const rhs (Set {random_type_collection()});

      BOOST_REQUIRE_EQUAL (rhs, assign_result (path, lhs, rhs));
    }

    BOOST_AUTO_TEST_CASE (assign_result_for_map_is_rhs)
    {
      Path const path (RandomPath{}());
      Type const lhs (Map {random_type_collection(), random_type_collection()});
      Type const rhs (Map {random_type_collection(), random_type_collection()});

      BOOST_REQUIRE_EQUAL (rhs, assign_result (path, lhs, rhs));
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_throws_when_types_of_field_are_different
      , testing::all_types() * testing::all_types()
      , tdA
      , tdB
      )
    {
      if (tdA.type != tdB.type)
      {
        auto const field_name (RandomIdentifier{}());
        Path const path (RandomPath{}());
        Path path_plus_field_name (path);
        path_plus_field_name.emplace_back (field_name);

        Type const lhs (Struct {{{field_name, tdA.type}}});
        Type const rhs (Struct {{{field_name, tdB.type}}});

        gspc::testing::require_exception
          ( [&]
            {
              (void) assign_result (path, lhs, rhs);
            }
          , exception::type::error
            { fmt::format
               ( "At {2}: Can not assign a value of type '{1}'"
                 " to a value of type '{0}'"
               , tdA.type
               , tdB.type
               , path_plus_field_name
               )
            }
          );
      }
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_throws_when_names_of_field_are_different
      , testing::all_types()
      , td
      )
    {
      gspc::testing::unique_random<Struct::Field::Name, RandomIdentifier>
        uniq_random_field_name;

      auto const field_nameA (uniq_random_field_name());
      auto const field_nameB (uniq_random_field_name());
      Path const path (RandomPath{}());
      Type const lhs (Struct {{{field_nameA, td.type}}});
      Type const rhs (Struct {{{field_nameB, td.type}}});

      gspc::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          { fmt::format
              ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Missing field '{3}', found '{4}' instead"
              , lhs
              , rhs
              , path
              , field_nameA
              , field_nameB
              )
          }
        );
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_throws_when_struct_fields_are_missing
      , testing::all_types()
      , td
      )
    {
      auto const field_name (RandomIdentifier{}());
      Path const path (RandomPath{}());
      Type const lhs (Struct {{{field_name, td.type}}});
      Type const rhs (Struct {{}});

      gspc::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          { fmt::format
              ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Missing field(s) {{'{3}'}}"
              , lhs
              , rhs
              , path
              , field_name
              )
          }
        );
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_throws_when_struct_has_additional_fields
      , testing::all_types()
      , td
      )
    {
      auto const field_name (RandomIdentifier{}());
      Path const path (RandomPath{}());
      Type const lhs (Struct {{}});
      Type const rhs (Struct {{{field_name, td.type}}});

      gspc::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          { fmt::format
              ( "Can not assign a value of type '{1}' to a value of type '{0}' at {2}: Additional field(s) {{'{3}'}}"
              , lhs
              , rhs
              , path
              , field_name
              )
          }
        );
    }
  }
