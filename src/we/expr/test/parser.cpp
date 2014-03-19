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

namespace
{
  template<typename T>
  void check_get_front (std::string const& expression, T const& value)
  {
    expr::eval::context context;

    BOOST_REQUIRE_EQUAL ( expr::parse::parser (expression, context).get_front()
                        , pnet::type::value::value_type (value)
                        );
  }
}

BOOST_AUTO_TEST_CASE (round_switches_between_half_up_and_half_down)
{
  check_get_front ("round (2.5)", 2.0);
  check_get_front ("round (2.5)", 3.0);
  check_get_front ("round (2.5)", 2.0);
  check_get_front ("round (2.5)", 3.0);
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
  check_get_front ("ceil (0.0)", 0.0);
  check_get_front ("ceil (0.25)", 1.0);
  check_get_front ("ceil (0.5)", 1.0);
  check_get_front ("ceil (0.75)", 1.0);
  check_get_front ("ceil (1.0)", 1.0);
  check_get_front ("ceil (1.25)", 2.0);
  check_get_front ("ceil (1.5)", 2.0);
  check_get_front ("ceil (1.75)", 2.0);
  check_get_front ("ceil (2.0)", 2.0);
}

BOOST_AUTO_TEST_CASE (_floor)
{
  check_get_front ("floor (0.0)", 0.0);
  check_get_front ("floor (0.25)", 0.0);
  check_get_front ("floor (0.5)", 0.0);
  check_get_front ("floor (0.75)", 0.0);
  check_get_front ("floor (1.0)", 1.0);
  check_get_front ("floor (1.25)", 1.0);
  check_get_front ("floor (1.5)", 1.0);
  check_get_front ("floor (1.75)", 1.0);
  check_get_front ("floor (2.0)", 2.0);
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
  check_get_front ("true || true", true);
  check_get_front ("true || false", true);
  check_get_front ("false || true", true);
  check_get_front ("false || false", false);

  check_get_front ("true :or: true", true);
  check_get_front ("true :or: false", true);
  check_get_front ("false :or: true", true);
  check_get_front ("false :or: false", false);

  check_get_front ("0 || 0", 0);
  check_get_front ("0 || 1", 1);
  check_get_front ("1 || 1", 1);
  check_get_front ("1 || 0", 1);
  check_get_front ("1 || 2", 3);
  check_get_front ("2 || 1", 3);

  check_get_front ("0U || 0U", 0U);
  check_get_front ("0U || 1U", 1U);
  check_get_front ("1U || 1U", 1U);
  check_get_front ("1U || 0U", 1U);
  check_get_front ("1U || 2U", 3U);
  check_get_front ("2U || 1U", 3U);

  check_get_front ("0L || 0L", 0L);
  check_get_front ("0L || 1L", 1L);
  check_get_front ("1L || 1L", 1L);
  check_get_front ("1L || 0L", 1L);
  check_get_front ("1L || 2L", 3L);
  check_get_front ("2L || 1L", 3L);

  check_get_front ("0UL || 0UL", 0UL);
  check_get_front ("0UL || 1UL", 1UL);
  check_get_front ("1UL || 1UL", 1UL);
  check_get_front ("1UL || 0UL", 1UL);
  check_get_front ("1UL || 2UL", 3UL);
  check_get_front ("2UL || 1UL", 3UL);
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
  check_get_front ("true && true", true);
  check_get_front ("true && false", false);
  check_get_front ("false && true", false);
  check_get_front ("false && false", false);

  check_get_front ("true :and: true", true);
  check_get_front ("true :and: false", false);
  check_get_front ("false :and: true", false);
  check_get_front ("false :and: false", false);

  check_get_front ("0 && 0", 0);
  check_get_front ("0 && 1", 0);
  check_get_front ("1 && 1", 1);
  check_get_front ("1 && 0", 0);
  check_get_front ("1 && 2", 0);
  check_get_front ("2 && 1", 0);

  check_get_front ("0U && 0U", 0U);
  check_get_front ("0U && 1U", 0U);
  check_get_front ("1U && 1U", 1U);
  check_get_front ("1U && 0U", 0U);
  check_get_front ("1U && 2U", 0U);
  check_get_front ("2U && 1U", 0U);

  check_get_front ("0L && 0L", 0L);
  check_get_front ("0L && 1L", 0L);
  check_get_front ("1L && 1L", 1L);
  check_get_front ("1L && 0L", 0L);
  check_get_front ("1L && 2L", 0L);
  check_get_front ("2L && 1L", 0L);

  check_get_front ("0UL && 0UL", 0UL);
  check_get_front ("0UL && 1UL", 0UL);
  check_get_front ("1UL && 1UL", 1UL);
  check_get_front ("1UL && 0UL", 0UL);
  check_get_front ("1UL && 2UL", 0UL);
  check_get_front ("2UL && 1UL", 0UL);
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
  check_get_front ("!true", false);
  check_get_front ("!false", true);
  check_get_front ("!!true", true);
  check_get_front ("!!false", false);
  check_get_front ("!0", true);
  check_get_front ("!1", false);
  check_get_front ("!0U", true);
  check_get_front ("!1U", false);
  check_get_front ("!0L", true);
  check_get_front ("!1L", false);
  check_get_front ("!0UL", true);
  check_get_front ("!1UL", false);
}

BOOST_AUTO_TEST_CASE (token_cmp)
{
#define CHECK(_lhs, _rhs, _lt, _le, _gt, _ge, _ne, _eq)                 \
  check_get_front ((boost::format ("%1% < %2%") % #_lhs % #_rhs).str(), _lt); \
  check_get_front ((boost::format ("%1% <= %2%") % #_lhs % #_rhs).str(), _le); \
  check_get_front ((boost::format ("%1% > %2%") % #_lhs % #_rhs).str(), _gt); \
  check_get_front ((boost::format ("%1% >= %2%") % #_lhs % #_rhs).str(), _ge); \
  check_get_front ((boost::format ("%1% != %2%") % #_lhs % #_rhs).str(), _ne); \
  check_get_front ((boost::format ("%1% == %2%") % #_lhs % #_rhs).str(), _eq)

  CHECK ('a', 'a', false, true, false, true, false, true);
  CHECK ('a', 'b', true, true, false, false, true, false);
  CHECK ('b', 'a', false, false, true, true, true, false);
  CHECK ("\"\"", "\"\"", false, true, false, true, false, true);
  CHECK ("\"\"", "\"a\"", true, true, false, false, true, false);
  CHECK ("\"a\"", "\"a\"", false, true, false, true, false, true);
  CHECK ("\"a\"", "\"b\"", true, true, false, false, true, false);
  CHECK ("\"a\"", "\"ab\"", true, true, false, false, true, false);
  CHECK ("\"a\"", "\"\"", false, false, true, true, true, false);
  CHECK ("\"b\"", "\"a\"", false, false, true, true, true, false);
  CHECK ("\"ab\"", "\"a\"", false, false, true, true, true, false);
  CHECK (true, true, false, true, false, true, false, true);
  CHECK (false, true, true, true, false, false, true, false);
  CHECK (true, false, false, false, true, true, true, false);
  CHECK (0, 0, false, true, false, true, false, true);
  CHECK (0, 1, true, true, false, false, true, false);
  CHECK (1, 0, false, false, true, true, true, false);
  CHECK (0U, 0U, false, true, false, true, false, true);
  CHECK (0U, 1U, true, true, false, false, true, false);
  CHECK (1U, 0U, false, false, true, true, true, false);
  CHECK (0L, 0L, false, true, false, true, false, true);
  CHECK (0L, 1L, true, true, false, false, true, false);
  CHECK (1L, 0L, false, false, true, true, true, false);
  CHECK (0UL, 0UL, false, true, false, true, false, true);
  CHECK (0UL, 1UL, true, true, false, false, true, false);
  CHECK (1UL, 0UL, false, false, true, true, true, false);
#undef CHECK

#define CHECK(_lhs, _rhs, _lt, _le, _gt, _ge)                           \
  check_get_front ((boost::format ("%1% < %2%") % #_lhs % #_rhs).str(), _lt); \
  check_get_front ((boost::format ("%1% <= %2%") % #_lhs % #_rhs).str(), _le); \
  check_get_front ((boost::format ("%1% > %2%") % #_lhs % #_rhs).str(), _gt); \
  check_get_front ((boost::format ("%1% >= %2%") % #_lhs % #_rhs).str(), _ge)

  CHECK (0.0, 0.0, false, true, false, true);
  CHECK (0.0, 1.0, true, true, false, false);
  CHECK (1.0, 0.0, false, false, true, true);
  CHECK (0.0f, 0.0f, false, true, false, true);
  CHECK (0.0f, 1.0f, true, true, false, false);
  CHECK (1.0f, 0.0f, false, false, true, true);
#undef CHECK

#define CHECK(_lhs, _rhs, _eq)                                          \
  check_get_front ((boost::format ("%1% == %2%") % #_lhs % #_rhs).str(), _eq)

  CHECK ({}, {}, true);
  CHECK ({}, bitset_insert {} 1L, false);
  CHECK (bitset_insert {} 1L, {}, false);
  CHECK (bitset_insert {} 1L, bitset_insert {} 2L, false);
  CHECK (bitset_insert (bitset_insert {} 1L) 2L, bitset_insert {} 2L, false);
  CHECK ( bitset_insert (bitset_insert {} 1L) 2L
        , bitset_insert (bitset_insert {} 2L) 1L, true
        );

  CHECK (y(), y(), true);
  CHECK (y(4), y(), false);
  CHECK (y(), y(4), false);
  CHECK (y(4), y(4), true);
#undef CHECK
}

BOOST_AUTO_TEST_CASE (token_add)
{
  check_get_front ("'a' + 'a'", std::string ("aa"));
  check_get_front ("\"\" + \"\"", std::string (""));
  check_get_front ("\"a\" + \"\"", std::string ("a"));
  check_get_front ("\"a\" + \"a\"", std::string ("aa"));
  check_get_front ("\"ab\" + \"a\"", std::string ("aba"));

#define CHECK_INTEGRAL(_type, _suffix)                                 \
  {                                                                    \
    _type const l (rand());                                            \
    _type const r (rand());                                            \
                                                                       \
    check_get_front ((boost::format ("%1%%3% + %2%%3%") % l % r % _suffix).str() \
                    , l + r                                             \
                    );                                                  \
  }

  CHECK_INTEGRAL (int, "");
  CHECK_INTEGRAL (unsigned int, "U");
  CHECK_INTEGRAL (long, "L");
  CHECK_INTEGRAL (unsigned long, "UL");
#undef CHECK_INTEGRAL

  check_get_front ("0.0 + 0.0", 0.0);
  check_get_front ("0.0 + 1.0", 1.0);
  check_get_front ("1.0 + 1.0", 2.0);
  check_get_front ("0.0f + 0.0f", 0.0f);
  check_get_front ("0.0f + 1.0f", 1.0f);
  check_get_front ("1.0f + 1.0f", 2.0f);
}

namespace
{
  void parser_ctor (std::string const& input)
  {
    (void)expr::parse::parser (input);
  }
}

BOOST_AUTO_TEST_CASE (token_sub)
{
#define CHECK_INTEGRAL(_type, _suffix)                                 \
  {                                                                    \
    _type const l (rand());                                            \
    _type const r (rand());                                            \
                                                                       \
    check_get_front ((boost::format ("%1%%3% - %2%%3%") % l % r % _suffix).str() \
                    , l - r                                             \
                    );                                                  \
  }

  CHECK_INTEGRAL (int, "");
  CHECK_INTEGRAL (long, "L");
#undef CHECK_INTEGRAL

  check_get_front ("0.0 - 0.0", 0.0);
  check_get_front ("0.0 - 1.0", -1.0);
  check_get_front ("1.0 - 1.0", 0.0);
  check_get_front ("0.0f - 0.0f", 0.0f);
  check_get_front ("0.0f - 1.0f", -1.0f);
  check_get_front ("1.0f - 1.0f", 0.0f);

  check_get_front ("0U - 0U", 0U);
  check_get_front ("1U - 0U", 1U);
  check_get_front ("2U - 1U", 1U);
  check_get_front ("0UL - 0UL", 0UL);
  check_get_front ("1UL - 0UL", 1UL);
  check_get_front ("2UL - 1UL", 1UL);

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0U - 1U")
    , "r > l => neg result"
    );

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0UL - 1UL")
    , "r > l => neg result"
    );
}
