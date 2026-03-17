// Copyright (C) 2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/memory_transfer.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/expr/type/Context.hpp>
#include <gspc/we/expr/type/testing/types.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <fmt/core.h>
#include <vector>

namespace
{
  gspc::we::expr::Type LocalRanges()
  {
    return gspc::we::expr::type::List
      ( gspc::we::expr::type::Struct
        ( { {"buffer", gspc::we::expr::type::String{}}
          , {"offset", gspc::we::expr::type::ULong{}}
          , {"size", gspc::we::expr::type::ULong{}}
          }
        )
      );
  }
  gspc::we::expr::Type GlobalRanges()
  {
    return gspc::we::expr::type::List
      ( gspc::we::expr::type::Struct
        ( { {"handle", gspc::we::expr::type::Struct ({{"name", gspc::we::expr::type::String{}}})}
          , {"offset", gspc::we::expr::type::ULong{}}
          , {"size", gspc::we::expr::type::ULong{}}
          }
        )
      );
  }

  template<typename Expression, typename Type, typename Expected>
    gspc::pnet::exception::type_error type_error
      ( Expression expression
      , Type type
      , Expected expected
      )
  {
    return fmt::format
      ( "Expression '{}' has incompatible type '{}'. Expected type '{}'."
      , expression
      , type
      , expected
      );
  }
}

BOOST_DATA_TEST_CASE
  ( local_expression_throws_when_not_of_type_List
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::List (gspc::we::expr::type::Any()))
  , td
  )
{
  gspc::we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  gspc::we::expr::type::Context context;
  context.bind ({"local"}, td.type);
  gspc::testing::require_exception
    ( [&]
      {
        memory_transfer.assert_correct_expression_types (context);
      }
    , gspc::testing::make_nested
      ( std::runtime_error ("In the <local> expression '${local}'")
      , type_error ("${local}", td.type, LocalRanges())
      )
    );
}

BOOST_DATA_TEST_CASE
  ( global_expression_throws_when_not_of_type_List
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::List (gspc::we::expr::type::Any()))
  , td
  )
{
  gspc::we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  gspc::we::expr::type::Context context;
  context.bind ({"local"}, LocalRanges());
  context.bind ({"global"}, td.type);
  gspc::testing::require_exception
    ( [&]
      {
        memory_transfer.assert_correct_expression_types (context);
      }
    , gspc::testing::make_nested
      ( std::runtime_error ("In the <global> expression '${global}'")
      , type_error ("${global}", td.type, GlobalRanges())
      )
    );
}

namespace
{
  std::vector<gspc::we::expr::Type> local_types()
  {
    return { gspc::we::expr::type::List {gspc::we::expr::type::Any()}
           , LocalRanges()
           , gspc::we::expr::type::List {gspc::we::expr::type::List {gspc::we::expr::type::Any()}}
           };
  }
  std::vector<gspc::we::expr::Type> global_types()
  {
    return { gspc::we::expr::type::List {gspc::we::expr::type::Any()}
           , GlobalRanges()
           , gspc::we::expr::type::List {gspc::we::expr::type::List {gspc::we::expr::type::Any()}}
           };
  }
}

BOOST_DATA_TEST_CASE
  ( local_and_global_of_type_list_are_accepted
  , local_types() * global_types()
  , local
  , global
  )
{
  gspc::we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  gspc::we::expr::type::Context context;
  context.bind ({"local"}, local);
  context.bind ({"global"}, global);

  BOOST_REQUIRE_NO_THROW
    (memory_transfer.assert_correct_expression_types (context));
}
