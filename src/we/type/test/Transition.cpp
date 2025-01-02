// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Transition.hpp>

#include <we/exception.hpp>
#include <we/expr/type/Context.hpp>
#include <we/expr/type/Type.hpp>
#include <we/expr/type/testing/types.hpp>
#include <we/type/net.hpp>
#include <we/type/signature.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <FMT/boost/variant.hpp>
#include <FMT/we/expr/type/Type.hpp>
#include <FMT/we/type/Expression.hpp>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
  template<typename Expression, typename Type>
    pnet::exception::type_error type_error
      ( Expression expression
      , Type type
      , expr::Type expected
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
  , expr::type::testing::all_types_except (expr::type::Boolean())
  , td
  )
{
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , we::type::Expression {td.expression}
    , we::type::property::type{}
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
        { fmt::format ("In the <condition> expression '{}'", td.expression)
        }
      , type_error (td.expression, td.type, expr::type::Boolean{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( expression_with_missing_binding_throws
  , expr::type::testing::all_types()
  , td
  )
{
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , we::type::property::type{}
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );
  auto const port_name (fhg::util::testing::random_identifier());
  transition.add_port
    ( we::type::Port
      ( port_name
      , we::type::port::direction::Out{}
      , td.signature
      , we::type::property::type{}
      )
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( pnet::exception::type_error ("In expression ''")
      , pnet::exception::missing_binding (port_name)
      )
    );
}

BOOST_DATA_TEST_CASE
  ( expression_with_wrong_output_type_throws
  , expr::type::testing::all_types() * expr::type::testing::all_types()
  , tdIn
  , tdOut
  )
{
  if (tdIn.signature != tdOut.signature)
  {
    auto const port_name (fhg::util::testing::random_identifier());
    we::type::Expression const expression
      {fmt::format ("${{{0}_out}} := ${{{0}_in}}", port_name)};
    we::type::Transition transition
      ( fhg::util::testing::random_identifier()
      , expression
      , ::boost::none // condition
      , we::type::property::type{}
      , we::priority_type{}
      , ::boost::none // eureka_id
      , std::list<we::type::Preference>{}
      );
    transition.add_port
      ( we::type::Port
        ( port_name + "_in"
        , we::type::port::direction::In{}
        , tdIn.signature
        , we::type::property::type{}
        )
      );
    transition.add_port
      ( we::type::Port
        ( port_name + "_out"
        , we::type::port::direction::Out{}
        , tdOut.signature
        , we::type::property::type{}
        )
      );

    fhg::util::testing::require_exception
      ( [&]
        {
          transition.assert_correct_expression_types();
        }
      , fhg::util::testing::make_nested
        ( pnet::exception::type_error
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
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "create"}, {});
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , pnet::exception::missing_binding ("plugin_path")
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_create_throws_when_plugin_path_has_not_type_String
  , expr::type::testing::all_types_except (expr::type::String())
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "create"}, {});
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );
  transition.add_port
    ( we::type::Port
      ( "plugin_path"
      , we::type::port::direction::In{}
      , td.signature
      , we::type::property::type{}
      )
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , pnet::exception::type_error
        { fmt::format
            ( "'plugin_path' has type '{}' but expected is type '{}'"
            , td.type
            , expr::type::String{}
            )
        }
    );
}

BOOST_AUTO_TEST_CASE (plugin_destroy_throws_when_plugin_id_is_not_set)
{
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "destroy"}, {});
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , pnet::exception::missing_binding ("plugin_id")
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_destroy_throws_when_plugin_id_has_not_type_ULong
  , expr::type::testing::all_types_except (expr::type::ULong())
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "destroy"}, {});
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );
  transition.add_port
    ( we::type::Port
      ( "plugin_id"
      , we::type::port::direction::In{}
      , td.signature
      , we::type::property::type{}
      )
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , pnet::exception::type_error
        { fmt::format
            ( "'plugin_id' has type '{}' but expected is type '{}'"
            , td.type
            , expr::type::ULong{}
            )
        }
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_call_before_eval_throws_when_expression_is_not_of_type_List_of_ULong
  , expr::type::testing::all_types_except (expr::type::List(expr::type::Any()))
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "call_before_eval"}, td.expression);
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
          ("In the property at 'gspc.we.plugin.call_before_eval'")
      , type_error
          (td.expression, td.type, expr::type::List (expr::type::ULong{}))
      )
    );
}

BOOST_DATA_TEST_CASE
  ( plugin_call_after_eval_throws_when_expression_is_not_of_type_List_of_ULong
  , expr::type::testing::all_types_except (expr::type::List(expr::type::Any()))
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"gspc", "we", "plugin", "call_after_eval"}, td.expression);
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
          ("In the property at 'gspc.we.plugin.call_after_eval'")
      , type_error
          (td.expression, td.type, expr::type::List (expr::type::ULong{}))
      )
    );
}

BOOST_DATA_TEST_CASE
  ( eureka_group_throws_when_expression_is_not_of_type_String
  , expr::type::testing::all_types_except (expr::type::String())
  , td
  )
{
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::ModuleCall{}
    , ::boost::none // condition
    , we::type::property::type{}
    , we::priority_type{}
    , td.expression
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
         { fmt::format ( "In the <eureka-group> expression '{}'"
                       , td.expression
                       )
         }
      , type_error (td.expression, td.type, expr::type::String{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( property_fhg_drts_schedule_num_worker_throws_when_not_of_type_ULong
  , expr::type::testing::all_types_except (expr::type::ULong())
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"fhg", "drts", "schedule", "num_worker"}, td.expression);
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
          ("In the property at 'fhg.drts.schedule.num_worker'")
      , type_error (td.expression, td.type, expr::type::ULong{})
      )
    );
}

BOOST_DATA_TEST_CASE
  ( property_fhg_drts_require_dynamic_requirement_throws_when_not_of_type_String
  , expr::type::testing::all_types_except (expr::type::String())
  , td
  )
{
  we::type::property::type properties;
  properties.set ({"fhg", "drts", "require", "dynamic_requirement"}, td.expression);
  we::type::Transition transition
    ( fhg::util::testing::random_identifier()
    , we::type::Expression{}
    , ::boost::none // condition
    , properties
    , we::priority_type{}
    , ::boost::none // eureka_id
    , std::list<we::type::Preference>{}
    );

  fhg::util::testing::require_exception
    ( [&]
      {
        transition.assert_correct_expression_types();
      }
    , fhg::util::testing::make_nested
      ( std::runtime_error
          ("In the property at 'fhg.drts.require.dynamic_requirement'")
      , type_error (td.expression, td.type, expr::type::String())
      )
    );
}
