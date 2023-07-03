// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/type/AssignResult.hpp>

#include <we/expr/exception.hpp>
#include <we/expr/type/Type.hpp>
#include <we/expr/type/testing/types.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/format.hpp>
#include <boost/test/data/test_case.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace expr
{
  namespace type
  {
    namespace
    {
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

      template<typename Container>
        Container random_selection (Container container)
      {
        fhg::util::testing::random<bool> random_bool;
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

        fhg::util::testing::require_exception
          ( [&]
            {
              (void) assign_result (path, tdA.type, tdB.type);
            }
          , exception::type::error
            ( ::boost::format
               ("At %3%: Can not assign a value of type '%2%'"
               " to a value of type '%1%'"
               )
            % tdA.type
            % tdB.type
            % path
            )
          );
      }
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

        fhg::util::testing::require_exception
          ( [&]
            {
              (void) assign_result (path, lhs, rhs);
            }
          , exception::type::error
            ( ::boost::format
               ("At %3%: Can not assign a value of type '%2%'"
               " to a value of type '%1%'"
               )
            % tdA.type
            % tdB.type
            % path_plus_field_name
            )
          );
      }
    }

    BOOST_DATA_TEST_CASE
      ( assign_result_throws_when_names_of_field_are_different
      , testing::all_types()
      , td
      )
    {
      fhg::util::testing::unique_random<Struct::Field::Name, RandomIdentifier>
        uniq_random_field_name;

      auto const field_nameA (uniq_random_field_name());
      auto const field_nameB (uniq_random_field_name());
      Path const path (RandomPath{}());
      Type const lhs (Struct {{{field_nameA, td.type}}});
      Type const rhs (Struct {{{field_nameB, td.type}}});

      fhg::util::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          ( ::boost::format
              ("Can not assign a value of type '%2%' to a value of type '%1%' at %3%: Missing field '%4%', found '%5%' instead")
          % lhs
          % rhs
          % path
          % field_nameA
          % field_nameB
          )
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

      fhg::util::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          ( ::boost::format
              ("Can not assign a value of type '%2%' to a value of type '%1%' at %3%: Missing field(s) {'%4%'}")
          % lhs
          % rhs
          % path
          % field_name
          )
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

      fhg::util::testing::require_exception
        ( [&]
          {
            (void) assign_result (path, lhs, rhs);
          }
        , exception::type::error
          ( ::boost::format
              ("Can not assign a value of type '%2%' to a value of type '%1%' at %3%: Additional field(s) {'%4%'}")
          % lhs
          % rhs
          % path
          % field_name
          )
        );
    }
  }
}
