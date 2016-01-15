// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser_performance
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
  fhg::util::testing::require_maximum_running_time<std::chrono::seconds>
    const maximum_running_time (1);

  const long round (750);
  const long max (2000);
  const std::string input ("${a} < ${b}");

  expr::eval::context context;

  context.bind ("b", max);

  expr::parse::parser parser (input);

  for (int r (0); r < round; ++r)
  {
    long i (0);

    do
    {
      context.bind ("a", i++);
    }
    while (pnet::type::value::is_true (parser.eval_all (context)));
  }
}

BOOST_AUTO_TEST_CASE (performance_often_parse_and_eval)
{
  fhg::util::testing::require_maximum_running_time<std::chrono::seconds>
    const maximum_running_time (1);

  const long round (75);
  const long max (2000);
  const std::string input ("${a} < ${b}");

  expr::eval::context context;

  context.bind ("b", max);

  for (int r (0); r < round; ++r)
  {
    long i (0);

    do
    {
      context.bind ("a", i++);
    }
    while (pnet::type::value::is_true (expr::parse::parser (input, context).get_front()));
  }
}
