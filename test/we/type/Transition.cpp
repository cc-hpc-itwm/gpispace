// Copyright (C) 2021-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Transition.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/expr/type/Context.hpp>
#include <gspc/we/expr/type/Type.hpp>
#include <gspc/we/expr/type/testing/types.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/signature.hpp>

#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <gspc/we/type/Expression.formatter.hpp>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  template<typename Expression, typename Type>
    gspc::pnet::exception::type_error type_error
      ( Expression expression
      , Type type
      , gspc::we::expr::Type expected
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
  ( condition_must_be_of_type_Boolean
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::Boolean())
  , td
  )
{
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , gspc::we::type::Expression {td.expression}
    , gspc::we::type::property::type{}
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
        { fmt::format ("In the <condition> expression '{}'", td.expression)
        }
      , type_error (td.expression, td.type, gspc::we::expr::type::Boolean{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( expression_with_missing_binding_throws
  , gspc::we::expr::type::testing::all_types()
  , td
  )
{
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , gspc::we::type::property::type{}
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  auto const port_name (gspc::testing::random_identifier());
  transition.add_port
    ( gspc::we::type::Port
      ( port_name
      , gspc::we::type::port::direction::Out{}
      , td.signature
      , gspc::we::type::property::type{}
      )
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( gspc::pnet::exception::type_error ("In expression ''")
      , gspc::pnet::exception::missing_binding (port_name)
      )
    );
}

BOOST_DATA_TEST_CASE
  ( expression_with_wrong_output_type_throws
  , gspc::we::expr::type::testing::all_types() * gspc::we::expr::type::testing::all_types()
  , tdIn
  , tdOut
  )
{
  if (tdIn.signature != tdOut.signature)
  {
    auto const port_name (gspc::testing::random_identifier());
    gspc::we::type::Expression const expression
      {fmt::format ("${{{0}_out}} := ${{{0}_in}}", port_name)};
    gspc::we::type::Transition transition
      ( gspc::testing::random_identifier()
      , expression
      , std::nullopt // condition
      , gspc::we::type::property::type{}
      , gspc::we::priority_type{}
      , std::nullopt // eureka_id
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    transition.add_port
      ( gspc::we::type::Port
        ( port_name + "_in"
        , gspc::we::type::port::direction::In{}
        , tdIn.signature
        , gspc::we::type::property::type{}
        )
      );
    transition.add_port
      ( gspc::we::type::Port
        ( port_name + "_out"
        , gspc::we::type::port::direction::Out{}
        , tdOut.signature
        , gspc::we::type::property::type{}
        )
      );

    gspc::testing::require_exception
      ( [&]
        {
          transition.assert_correct_expression_types();
        }
      , gspc::testing::make_nested
        ( gspc::pnet::exception::type_error
            { fmt::format ("In expression '{}'", expression)
            }
        , std::runtime_error
            { fmt::format ( "Output port '{}' expects type '{}'"
                          , port_name + "_out"
                          , tdOut.type
                          )
            }
        )
      );
  }
}

BOOST_AUTO_TEST_CASE (plugin_create_throws_when_plugin_path_is_not_set)
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "create"}, {});
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::pnet::exception::missing_binding ("plugin_path")
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_create_throws_when_plugin_path_has_not_type_String
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::String())
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "create"}, {});
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  transition.add_port
    ( gspc::we::type::Port
      ( "plugin_path"
      , gspc::we::type::port::direction::In{}
      , td.signature
      , gspc::we::type::property::type{}
      )
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::pnet::exception::type_error
        { fmt::format
            ( "'plugin_path' has type '{}' but expected is type '{}'"
            , td.type
            , gspc::we::expr::type::String{}
            )
        }
    );
}

BOOST_AUTO_TEST_CASE (plugin_destroy_throws_when_plugin_id_is_not_set)
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "destroy"}, {});
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::pnet::exception::missing_binding ("plugin_id")
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_destroy_throws_when_plugin_id_has_not_type_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::ULong())
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "destroy"}, {});
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  transition.add_port
    ( gspc::we::type::Port
      ( "plugin_id"
      , gspc::we::type::port::direction::In{}
      , td.signature
      , gspc::we::type::property::type{}
      )
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::pnet::exception::type_error
        { fmt::format
            ( "'plugin_id' has type '{}' but expected is type '{}'"
            , td.type
            , gspc::we::expr::type::ULong{}
            )
        }
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_call_before_eval_throws_when_expression_is_not_of_type_List_of_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::List(gspc::we::expr::type::Any()))
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "call_before_eval"}, td.expression);
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
          ("In the property at 'gspc.we.plugin.call_before_eval'")
      , type_error
          (td.expression, td.type, gspc::we::expr::type::List (gspc::we::expr::type::ULong{}))
      )
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_call_after_eval_throws_when_expression_is_not_of_type_List_of_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::List(gspc::we::expr::type::Any()))
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "call_after_eval"}, td.expression);
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
          ("In the property at 'gspc.we.plugin.call_after_eval'")
      , type_error
          (td.expression, td.type, gspc::we::expr::type::List (gspc::we::expr::type::ULong{}))
      )
    );
}

BOOST_DATA_TEST_CASE
  ( eureka_group_throws_when_expression_is_not_of_type_String
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::String())
  , td
  )
{
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::ModuleCall{}
    , std::nullopt // condition
    , gspc::we::type::property::type{}
    , gspc::we::priority_type{}
    , td.expression
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
         { fmt::format ( "In the <eureka-group> expression '{}'"
                       , td.expression
                       )
         }
      , type_error (td.expression, td.type, gspc::we::expr::type::String{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( property_fhg_drts_schedule_num_worker_throws_when_not_of_type_ULong
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::ULong())
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"fhg", "drts", "schedule", "num_worker"}, td.expression);
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
          ("In the property at 'fhg.drts.schedule.num_worker'")
      , type_error (td.expression, td.type, gspc::we::expr::type::ULong{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( property_fhg_drts_require_dynamic_requirement_throws_when_not_of_type_String
  , gspc::we::expr::type::testing::all_types_except (gspc::we::expr::type::String())
  , td
  )
{
  gspc::we::type::property::type properties;
  properties.set ({"fhg", "drts", "require", "dynamic_requirement"}, td.expression);
  gspc::we::type::Transition transition
    ( gspc::testing::random_identifier()
    , gspc::we::type::Expression{}
    , std::nullopt // condition
    , properties
    , gspc::we::priority_type{}
    , std::nullopt // eureka_id
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );

  gspc::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , gspc::testing::make_nested
      ( std::runtime_error
          ("In the property at 'fhg.drts.require.dynamic_requirement'")
      , type_error (td.expression, td.type, gspc::we::expr::type::String())
      )
    );
}
