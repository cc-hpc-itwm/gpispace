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

#include <boost/test/unit_test.hpp>

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value/function.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <functional>
#include <limits>
#include <random>
#include <string>
#include <stack>

BOOST_AUTO_TEST_CASE (performance_parse_once_eval_often)
{
  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    const long round (750);
    const pnet::type::value::value_type max (2000L);
    const std::string input ("${a} < ${b}");

    expr::eval::context context;

    context.bind_ref ("b", max);

    expr::parse::parser parser (input);

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
  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    const long round (75);
    const pnet::type::value::value_type max (2000L);
    const std::string input ("${a} < ${b}");

    expr::eval::context context;

    context.bind_ref ("b", max);

    for (int r (0); r < round; ++r)
    {
      long i (0);

      do
      {
        context.bind_and_discard_ref ({"a"}, i++);
      }
      while (::boost::get<bool> (expr::parse::parser (input).eval_all (context)));
    }
  };
}
