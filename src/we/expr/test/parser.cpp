// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser
#include <boost/test/unit_test.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <string>

#ifdef NDEBUG
#include <fhg/util/now.hpp>

BOOST_AUTO_TEST_CASE (performance_parse_once_eval_often)
{
  double const t (-fhg::util::now());

  const long round (750);
  const long max (3000);
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
    while (parser.eval_front_bool (context));
  }

  BOOST_REQUIRE_LT (t + fhg::util::now(), 1.0);
}

BOOST_AUTO_TEST_CASE (performance_often_parse_and_eval)
{
  double const t (-fhg::util::now());

  const long round (75);
  const long max (3000);
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
    while (expr::parse::parser (input, context).get_front_bool());
  }

  BOOST_REQUIRE_LT (t + fhg::util::now(), 1.0);
}
#endif

BOOST_AUTO_TEST_CASE (round_switches_between_half_up_and_half_down)
{
  const std::string input ("round (2.5)");
  expr::eval::context context;

#define CHECK(v)                                                         \
  BOOST_REQUIRE_EQUAL ( expr::parse::parser (input, context).get_front() \
                      , pnet::type::value::value_type (v)                \
                      )

  CHECK (2.0);
  CHECK (3.0);
  CHECK (2.0);
  CHECK (3.0);
#undef CHECK
}

BOOST_AUTO_TEST_CASE (comment)
{
 BOOST_REQUIRE_EQUAL
   ( expr::parse::parser ("").string()
   , expr::parse::parser ("/**/").string()
   );
 BOOST_REQUIRE_EQUAL
   ( expr::parse::parser ("").string()
   , expr::parse::parser ("/* something in between */").string()
   );
}
BOOST_AUTO_TEST_CASE (nested_comment)
{
 BOOST_REQUIRE_EQUAL
   ( expr::parse::parser ("").string()
   , expr::parse::parser ("/*/**/*/").string()
   );
 BOOST_REQUIRE_EQUAL
   ( expr::parse::parser ("").string()
   , expr::parse::parser ("/* something at level 1 /**/ */").string()
   );
}

BOOST_AUTO_TEST_CASE (parens_can_be_omitted_after_floor)
{
  BOOST_REQUIRE_EQUAL
   ( expr::parse::parser ("floor ${a}").string()
   , expr::parse::parser ("floor (${a})").string()
   );
}

BOOST_AUTO_TEST_CASE (ceiling)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression, context).get_front()          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("ceil (0.0)", 0.0);
  CHECK ("ceil (0.25)", 1.0);
  CHECK ("ceil (0.5)", 1.0);
  CHECK ("ceil (0.75)", 1.0);
  CHECK ("ceil (1.0)", 1.0);
  CHECK ("ceil (1.25)", 2.0);
  CHECK ("ceil (1.5)", 2.0);
  CHECK ("ceil (1.75)", 2.0);
  CHECK ("ceil (2.0)", 2.0);
#undef CHECK
}

BOOST_AUTO_TEST_CASE (_floor)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression, context).get_front()          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("floor (0.0)", 0.0);
  CHECK ("floor (0.25)", 0.0);
  CHECK ("floor (0.5)", 0.0);
  CHECK ("floor (0.75)", 0.0);
  CHECK ("floor (1.0)", 1.0);
  CHECK ("floor (1.25)", 1.0);
  CHECK ("floor (1.5)", 1.0);
  CHECK ("floor (1.75)", 1.0);
  CHECK ("floor (2.0)", 2.0);
#undef CHECK
}

BOOST_AUTO_TEST_CASE (other_variable_not_renamed)
{
  expr::parse::parser parser ("${_}");
  parser.rename ("a", "A");

  BOOST_REQUIRE_EQUAL (parser.string(), expr::parse::parser ("${_}").string());
}

BOOST_AUTO_TEST_CASE (variable_renamed)
{
  expr::parse::parser parser ("${a}");
  parser.rename ("a", "A");

  BOOST_REQUIRE_EQUAL (parser.string(), expr::parse::parser ("${A}").string());
}

BOOST_AUTO_TEST_CASE (first_field_renamed)
{
  expr::parse::parser parser ("${a._}");
  parser.rename ("a", "A");

  BOOST_REQUIRE_EQUAL
    (parser.string(), expr::parse::parser ("${A._}").string());
}

BOOST_AUTO_TEST_CASE (later_field_not_renamed)
{
  expr::parse::parser parser ("${_.a}");
  parser.rename ("a", "A");

  BOOST_REQUIRE_EQUAL
    (parser.string(), expr::parse::parser ("${_.a}").string());
}

BOOST_AUTO_TEST_CASE (renamed_at_depth_greater_zero)
{
  expr::parse::parser parser ("sin (${a})");
  parser.rename ("a", "A");

  BOOST_REQUIRE_EQUAL
    (parser.string(), expr::parse::parser ("sin (${A})").string());
}
