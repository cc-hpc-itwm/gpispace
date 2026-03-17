// Copyright (C) 2014-2016,2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/expr/parse/parser.hpp>
#include <gspc/testing/printer/we/type/value.hpp>
#include <gspc/we/type/value/function.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_maximum_running_time.hpp>

#include <functional>
#include <limits>
#include <random>
#include <stack>
#include <string>

BOOST_AUTO_TEST_CASE (performance_parse_once_eval_often)
{
  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    const long round (750);
    const gspc::pnet::type::value::value_type max (2000L);
    const std::string input ("${a} < ${b}");

    gspc::we::expr::eval::context context;

    context.bind_ref ("b", max);

    gspc::we::expr::parse::parser parser (input);

    for (int r (0); r < round; ++r)
    {
      long i (0);

      do
      {
        context.bind_and_discard_ref ({"a"}, i++);
      }
      while (::boost::get<bool> (parser.eval_all (context)));
    }
  };
}

BOOST_AUTO_TEST_CASE (performance_often_parse_and_eval)
{
  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    const long round (75);
    const gspc::pnet::type::value::value_type max (2000L);
    const std::string input ("${a} < ${b}");

    gspc::we::expr::eval::context context;

    context.bind_ref ("b", max);

    for (int r (0); r < round; ++r)
    {
      long i (0);

      do
      {
        context.bind_and_discard_ref ({"a"}, i++);
      }
      while (::boost::get<bool> (gspc::we::expr::parse::parser (input).eval_all (context)));
    }
  };
}
