// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser
#include <boost/test/unit_test.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>

#include <we/exception.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>

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

BOOST_AUTO_TEST_CASE (token_or_table)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression, context).get_front()          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("true || true", true);
  CHECK ("true || false", true);
  CHECK ("false || true", true);
  CHECK ("false || false", false);

  CHECK ("true :or: true", true);
  CHECK ("true :or: false", true);
  CHECK ("false :or: true", true);
  CHECK ("false :or: false", false);

  CHECK ("0 || 0", 0);
  CHECK ("0 || 1", 1);
  CHECK ("1 || 1", 1);
  CHECK ("1 || 0", 1);
  CHECK ("1 || 2", 3);
  CHECK ("2 || 1", 3);

  CHECK ("0U || 0U", 0U);
  CHECK ("0U || 1U", 1U);
  CHECK ("1U || 1U", 1U);
  CHECK ("1U || 0U", 1U);
  CHECK ("1U || 2U", 3U);
  CHECK ("2U || 1U", 3U);

  CHECK ("0L || 0L", 0L);
  CHECK ("0L || 1L", 1L);
  CHECK ("1L || 1L", 1L);
  CHECK ("1L || 0L", 1L);
  CHECK ("1L || 2L", 3L);
  CHECK ("2L || 1L", 3L);

  CHECK ("0UL || 0UL", 0UL);
  CHECK ("0UL || 1UL", 1UL);
  CHECK ("1UL || 1UL", 1UL);
  CHECK ("1UL || 0UL", 1UL);
  CHECK ("1UL || 2UL", 3UL);
  CHECK ("2UL || 1UL", 3UL);
#undef CHECK
}

namespace
{
  void get_and_ignore_value_from_context
    (expr::eval::context const& context, std::string const& key)
  {
    (void)context.value (key);
  }
}

BOOST_AUTO_TEST_CASE (token_or_short_circuit)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression).eval_front (context)          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("true || (${a} := true)", true);

  fhg::util::boost::test::require_exception<pnet::exception::missing_binding>
    ( boost::bind (&get_and_ignore_value_from_context, context, "a")
    , "missing binding for: ${a}"
    );

  CHECK ("false || (${a} := true)", true);

  BOOST_REQUIRE_EQUAL
    (context.value ("a"), pnet::type::value::value_type (true));

#undef CHECK
}

BOOST_AUTO_TEST_CASE (token_and_table)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression, context).get_front()          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("true && true", true);
  CHECK ("true && false", false);
  CHECK ("false && true", false);
  CHECK ("false && false", false);

  CHECK ("true :and: true", true);
  CHECK ("true :and: false", false);
  CHECK ("false :and: true", false);
  CHECK ("false :and: false", false);

  CHECK ("0 && 0", 0);
  CHECK ("0 && 1", 0);
  CHECK ("1 && 1", 1);
  CHECK ("1 && 0", 0);
  CHECK ("1 && 2", 0);
  CHECK ("2 && 1", 0);

  CHECK ("0U && 0U", 0U);
  CHECK ("0U && 1U", 0U);
  CHECK ("1U && 1U", 1U);
  CHECK ("1U && 0U", 0U);
  CHECK ("1U && 2U", 0U);
  CHECK ("2U && 1U", 0U);

  CHECK ("0L && 0L", 0L);
  CHECK ("0L && 1L", 0L);
  CHECK ("1L && 1L", 1L);
  CHECK ("1L && 0L", 0L);
  CHECK ("1L && 2L", 0L);
  CHECK ("2L && 1L", 0L);

  CHECK ("0UL && 0UL", 0UL);
  CHECK ("0UL && 1UL", 0UL);
  CHECK ("1UL && 1UL", 1UL);
  CHECK ("1UL && 0UL", 0UL);
  CHECK ("1UL && 2UL", 0UL);
  CHECK ("2UL && 1UL", 0UL);
#undef CHECK
}

BOOST_AUTO_TEST_CASE (token_and_short_circuit)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression).eval_front (context)          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("false && (${a} := false)", false);

  fhg::util::boost::test::require_exception<pnet::exception::missing_binding>
    ( boost::bind (&get_and_ignore_value_from_context, context, "a")
    , "missing binding for: ${a}"
    );

  CHECK ("true && (${a} := false)", false);

  BOOST_REQUIRE_EQUAL
    (context.value ("a"), pnet::type::value::value_type (false));

#undef CHECK
}

BOOST_AUTO_TEST_CASE (token_not)
{
  expr::eval::context context;

#define CHECK(_expression, _value)                                    \
  BOOST_REQUIRE_EQUAL                                                 \
    ( expr::parse::parser (_expression).eval_front (context)          \
    , pnet::type::value::value_type (_value)                          \
    )

  CHECK ("!true", false);
  CHECK ("!false", true);
  CHECK ("!!true", true);
  CHECK ("!!false", false);
  CHECK ("!0", true);
  CHECK ("!1", false);
  CHECK ("!0U", true);
  CHECK ("!1U", false);
  CHECK ("!0L", true);
  CHECK ("!1L", false);
  CHECK ("!0UL", true);
  CHECK ("!1UL", false);

#undef CHECK
}
