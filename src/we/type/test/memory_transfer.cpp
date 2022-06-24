// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/type/memory_transfer.hpp>

#include <we/exception.hpp>
#include <we/expr/type/Context.hpp>
#include <we/expr/type/testing/types.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <vector>

namespace
{
  expr::Type LocalRanges()
  {
    return expr::type::List
      ( expr::type::Struct
        ( { {"buffer", expr::type::String{}}
          , {"offset", expr::type::ULong{}}
          , {"size", expr::type::ULong{}}
          }
        )
      );
  }
  expr::Type GlobalRanges()
  {
    return expr::type::List
      ( expr::type::Struct
        ( { {"handle", expr::type::Struct ({{"name", expr::type::String{}}})}
          , {"offset", expr::type::ULong{}}
          , {"size", expr::type::ULong{}}
          }
        )
      );
  }

  template<typename Expression, typename Type, typename Expected>
    pnet::exception::type_error type_error
      ( Expression expression
      , Type type
      , Expected expected
      )
  {
    return str
      ( ::boost::format
          ("Expression '%1%' has incompatible type '%2%'."
          " Expected type '%3%'."
          )
      % expression
      % type
      % expected
      );
  }
}

BOOST_DATA_TEST_CASE
  ( local_expression_throws_when_not_of_type_List
  , expr::type::testing::all_types_except (expr::type::List (expr::type::Any()))
  , td
  )
{
  we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  expr::type::Context context;
  context.bind ({"local"}, td.type);
  fhg::util::testing::require_exception
    ( [&]
      {
        memory_transfer.assert_correct_expression_types (context);
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error ("In the <local> expression '${local}'")
      , type_error ("${local}", td.type, LocalRanges())
      )
    );
}

BOOST_DATA_TEST_CASE
  ( global_expression_throws_when_not_of_type_List
  , expr::type::testing::all_types_except (expr::type::List (expr::type::Any()))
  , td
  )
{
  we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  expr::type::Context context;
  context.bind ({"local"}, LocalRanges());
  context.bind ({"global"}, td.type);
  fhg::util::testing::require_exception
    ( [&]
      {
        memory_transfer.assert_correct_expression_types (context);
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error ("In the <global> expression '${global}'")
      , type_error ("${global}", td.type, GlobalRanges())
      )
    );
}

namespace
{
  std::vector<expr::Type> local_types()
  {
    return { expr::type::List {expr::type::Any()}
           , LocalRanges()
           , expr::type::List {expr::type::List {expr::type::Any()}}
           };
  }
  std::vector<expr::Type> global_types()
  {
    return { expr::type::List {expr::type::Any()}
           , GlobalRanges()
           , expr::type::List {expr::type::List {expr::type::Any()}}
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
  we::type::memory_transfer const memory_transfer
    ("${global}", "${local}", {}, {});

  expr::type::Context context;
  context.bind ({"local"}, local);
  context.bind ({"global"}, global);

  BOOST_REQUIRE_NO_THROW
    (memory_transfer.assert_correct_expression_types (context));
}
