// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser
#include <boost/test/unit_test.hpp>

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/read.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/random_string.hpp>

#include <functional>
#include <limits>
#include <random>
#include <string>
#include <stack>

namespace
{
  template<typename T>
  expr::eval::context require_evaluating_to
    (std::string const& expression, T const& value)
  {
    expr::eval::context context;

    BOOST_REQUIRE_EQUAL ( expr::parse::parser (expression).eval_front (context)
                        , pnet::type::value::value_type (value)
                        );

    return context;
  }

  template<typename T>
  expr::eval::context require_evaluating_to
    (boost::format const& format, T const& value)
  {
    return require_evaluating_to (format.str(), value);
  }
}

BOOST_AUTO_TEST_CASE (round_is_away_from_zero)
{
  require_evaluating_to ("round (2.5)", 3.0);
  require_evaluating_to ("round (2.5)", 3.0);
  require_evaluating_to ("round (-2.5)", -3.0);
  require_evaluating_to ("round (-2.5)", -3.0);
  require_evaluating_to ("round (2.5f)", 3.0f);
  require_evaluating_to ("round (2.5f)", 3.0f);
  require_evaluating_to ("round (-2.5f)", -3.0f);
  require_evaluating_to ("round (-2.5f)", -3.0f);
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
  require_evaluating_to ("ceil (0.0)", 0.0);
  require_evaluating_to ("ceil (0.25)", 1.0);
  require_evaluating_to ("ceil (0.5)", 1.0);
  require_evaluating_to ("ceil (0.75)", 1.0);
  require_evaluating_to ("ceil (1.0)", 1.0);
  require_evaluating_to ("ceil (1.25)", 2.0);
  require_evaluating_to ("ceil (1.5)", 2.0);
  require_evaluating_to ("ceil (1.75)", 2.0);
  require_evaluating_to ("ceil (2.0)", 2.0);
}

BOOST_AUTO_TEST_CASE (_floor)
{
  require_evaluating_to ("floor (0.0)", 0.0);
  require_evaluating_to ("floor (0.25)", 0.0);
  require_evaluating_to ("floor (0.5)", 0.0);
  require_evaluating_to ("floor (0.75)", 0.0);
  require_evaluating_to ("floor (1.0)", 1.0);
  require_evaluating_to ("floor (1.25)", 1.0);
  require_evaluating_to ("floor (1.5)", 1.0);
  require_evaluating_to ("floor (1.75)", 1.0);
  require_evaluating_to ("floor (2.0)", 2.0);
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

BOOST_AUTO_TEST_CASE (token_or_boolean_table)
{
  require_evaluating_to ("true || true", true);
  require_evaluating_to ("true || false", true);
  require_evaluating_to ("false || true", true);
  require_evaluating_to ("false || false", false);

  require_evaluating_to ("true :or: true", true);
  require_evaluating_to ("true :or: false", true);
  require_evaluating_to ("false :or: true", true);
  require_evaluating_to ("false :or: false", false);
}

BOOST_AUTO_TEST_CASE (token_or_integral)
{
  require_evaluating_to ("0 | 0", 0);
  require_evaluating_to ("0 | 1", 1);
  require_evaluating_to ("1 | 1", 1);
  require_evaluating_to ("1 | 0", 1);
  require_evaluating_to ("1 | 2", 3);
  require_evaluating_to ("2 | 1", 3);

  require_evaluating_to ("0U | 0U", 0U);
  require_evaluating_to ("0U | 1U", 1U);
  require_evaluating_to ("1U | 1U", 1U);
  require_evaluating_to ("1U | 0U", 1U);
  require_evaluating_to ("1U | 2U", 3U);
  require_evaluating_to ("2U | 1U", 3U);

  require_evaluating_to ("0L | 0L", 0L);
  require_evaluating_to ("0L | 1L", 1L);
  require_evaluating_to ("1L | 1L", 1L);
  require_evaluating_to ("1L | 0L", 1L);
  require_evaluating_to ("1L | 2L", 3L);
  require_evaluating_to ("2L | 1L", 3L);

  require_evaluating_to ("0UL | 0UL", 0UL);
  require_evaluating_to ("0UL | 1UL", 1UL);
  require_evaluating_to ("1UL | 1UL", 1UL);
  require_evaluating_to ("1UL | 0UL", 1UL);
  require_evaluating_to ("1UL | 2UL", 3UL);
  require_evaluating_to ("2UL | 1UL", 3UL);
}

BOOST_AUTO_TEST_CASE (token_or_short_circuit)
{
  fhg::util::testing::require_exception
    ( [] { require_evaluating_to ("true || (${a} := true)", true).value ("a"); }
    , pnet::exception::missing_binding ("a")
    );

  BOOST_REQUIRE_EQUAL
    ( require_evaluating_to ("false || (${a} := true)", true).value ("a")
    , pnet::type::value::value_type (true)
    );
}

BOOST_AUTO_TEST_CASE (token_and_boolean_table)
{
  require_evaluating_to ("true && true", true);
  require_evaluating_to ("true && false", false);
  require_evaluating_to ("false && true", false);
  require_evaluating_to ("false && false", false);

  require_evaluating_to ("true :and: true", true);
  require_evaluating_to ("true :and: false", false);
  require_evaluating_to ("false :and: true", false);
  require_evaluating_to ("false :and: false", false);
}

BOOST_AUTO_TEST_CASE (token_and_integral)
{
  require_evaluating_to ("0 & 0", 0);
  require_evaluating_to ("0 & 1", 0);
  require_evaluating_to ("1 & 1", 1);
  require_evaluating_to ("1 & 0", 0);
  require_evaluating_to ("1 & 2", 0);
  require_evaluating_to ("2 & 1", 0);
  require_evaluating_to ("2 & 3", 2);

  require_evaluating_to ("0U & 0U", 0U);
  require_evaluating_to ("0U & 1U", 0U);
  require_evaluating_to ("1U & 1U", 1U);
  require_evaluating_to ("1U & 0U", 0U);
  require_evaluating_to ("1U & 2U", 0U);
  require_evaluating_to ("2U & 1U", 0U);
  require_evaluating_to ("2U & 3U", 2U);

  require_evaluating_to ("0L & 0L", 0L);
  require_evaluating_to ("0L & 1L", 0L);
  require_evaluating_to ("1L & 1L", 1L);
  require_evaluating_to ("1L & 0L", 0L);
  require_evaluating_to ("1L & 2L", 0L);
  require_evaluating_to ("2L & 1L", 0L);
  require_evaluating_to ("2L & 3L", 2L);

  require_evaluating_to ("0UL & 0UL", 0UL);
  require_evaluating_to ("0UL & 1UL", 0UL);
  require_evaluating_to ("1UL & 1UL", 1UL);
  require_evaluating_to ("1UL & 0UL", 0UL);
  require_evaluating_to ("1UL & 2UL", 0UL);
  require_evaluating_to ("2UL & 1UL", 0UL);
  require_evaluating_to ("2UL & 3UL", 2UL);
}

BOOST_AUTO_TEST_CASE (token_and_short_circuit)
{
  fhg::util::testing::require_exception
    ( [] { require_evaluating_to ("false && (${a} := false)", false).value ("a"); }
    , pnet::exception::missing_binding ("a")
    );

  BOOST_REQUIRE_EQUAL
    ( require_evaluating_to ("true && (${a} := false)", false).value ("a")
    , pnet::type::value::value_type (false)
    );
}

BOOST_AUTO_TEST_CASE (token_not)
{
  require_evaluating_to ("!true", false);
  require_evaluating_to ("!false", true);
  require_evaluating_to ("!!true", true);
  require_evaluating_to ("!!false", false);
  require_evaluating_to ("!0", ~0);
  require_evaluating_to ("!1", ~1);
  require_evaluating_to ("!0U", ~0U);
  require_evaluating_to ("!1U", ~1U);
  require_evaluating_to ("!0L", ~0L);
  require_evaluating_to ("!1L", ~1L);
  require_evaluating_to ("!0UL", ~0UL);
  require_evaluating_to ("!1UL", ~1UL);
}

namespace
{
  void check_compare
    (std::string lhs, std::string rhs, bool lt, bool le, bool gt, bool ge)
  {
    require_evaluating_to (boost::format ("%1% < %2%") % lhs % rhs, lt);
    require_evaluating_to (boost::format ("%1% <= %2%") % lhs % rhs, le);
    require_evaluating_to (boost::format ("%1% > %2%") % lhs % rhs, gt);
    require_evaluating_to (boost::format ("%1% >= %2%") % lhs % rhs, ge);

    require_evaluating_to (boost::format ("%1% :lt: %2%") % lhs % rhs, lt);
    require_evaluating_to (boost::format ("%1% :le: %2%") % lhs % rhs, le);
    require_evaluating_to (boost::format ("%1% :gt: %2%") % lhs % rhs, gt);
    require_evaluating_to (boost::format ("%1% :ge: %2%") % lhs % rhs, ge);
  }

  void check_equality (std::string lhs, std::string rhs, bool eq)
  {
    require_evaluating_to (boost::format ("%1% != %2%") % lhs % rhs, !eq);
    require_evaluating_to (boost::format ("%1% == %2%") % lhs % rhs, eq);

    require_evaluating_to (boost::format ("%1% :ne: %2%") % lhs % rhs, !eq);
    require_evaluating_to (boost::format ("%1% :eq: %2%") % lhs % rhs, eq);
  }
}

namespace
{
  template<typename Exception>
    void require_ctor_exception
    (std::string const& input, Exception exception)
  {
    fhg::util::testing::require_exception
      ([&input]() { (void)expr::parse::parser (input); }, exception);
  }
}

BOOST_AUTO_TEST_CASE (token_cmp)
{
#define CHECK(_lhs, _rhs, _lt, _le, _gt, _ge, _eq)                      \
  check_compare (#_lhs, #_rhs, _lt, _le, _gt, _ge);                     \
  check_equality (#_lhs, #_rhs, _eq)

  CHECK ('a', 'a', false, true, false, true, true);
  CHECK ('a', 'b', true, true, false, false, false);
  CHECK ('b', 'a', false, false, true, true, false);
  CHECK ("\"\"", "\"\"", false, true, false, true, true);
  CHECK ("\"\"", "\"a\"", true, true, false, false, false);
  CHECK ("\"a\"", "\"a\"", false, true, false, true, true);
  CHECK ("\"a\"", "\"b\"", true, true, false, false, false);
  CHECK ("\"a\"", "\"ab\"", true, true, false, false, false);
  CHECK ("\"a\"", "\"\"", false, false, true, true, false);
  CHECK ("\"b\"", "\"a\"", false, false, true, true, false);
  CHECK ("\"ab\"", "\"a\"", false, false, true, true, false);
  CHECK (true, true, false, true, false, true, true);
  CHECK (false, true, true, true, false, false, false);
  CHECK (true, false, false, false, true, true, false);
  CHECK (0, 0, false, true, false, true, true);
  CHECK (0, 1, true, true, false, false, false);
  CHECK (1, 0, false, false, true, true, false);
  CHECK (0U, 0U, false, true, false, true, true);
  CHECK (0U, 1U, true, true, false, false, false);
  CHECK (1U, 0U, false, false, true, true, false);
  CHECK (0L, 0L, false, true, false, true, true);
  CHECK (0L, 1L, true, true, false, false, false);
  CHECK (1L, 0L, false, false, true, true, false);
  CHECK (0UL, 0UL, false, true, false, true, true);
  CHECK (0UL, 1UL, true, true, false, false, false);
  CHECK (1UL, 0UL, false, false, true, true, false);
#undef CHECK

  check_compare ("0.0", "0.0", false, true, false, true);
  check_compare ("0.0", "1.0", true, true, false, false);
  check_compare ("1.0", "0.0", false, false, true, true);
  check_compare ("0.0f", "0.0f", false, true, false, true);
  check_compare ("0.0f", "1.0f", true, true, false, false);
  check_compare ("1.0f", "0.0f", false, false, true, true);

  check_equality ("{}", "{}", true);
  check_equality ("{}", "bitset_insert {} 1UL", false);
  check_equality ("bitset_insert {} 1UL", "{}", false);
  check_equality ("bitset_insert {} 1UL", "bitset_insert {} 2UL", false);
  check_equality ( "bitset_insert (bitset_insert {} 1UL) 2UL"
                 , "bitset_insert {} 2UL", false
                 );
  check_equality ( "bitset_insert (bitset_insert {} 1UL) 2UL"
                 , "bitset_insert (bitset_insert {} 2UL) 1UL", true
                 );

  check_equality ("y()", "y()", true);
  check_equality ("y(4)", "y()", false);
  check_equality ("y()", "y(4)", false);
  check_equality ("y(4)", "y(4)", true);

  check_equality ("Struct[]", "Struct[]", true);
  check_equality ("Struct[a:=0]", "Struct[a:=0]", true);
  check_equality ("Struct[a:=0]", "Struct[a:=1]", false);

  require_ctor_exception
    ( "Struct [a:=0] == Struct [b:=0]"
    , pnet::exception::eval ( expr::token::type::eq
                            , pnet::type::value::read ("Struct [a := 0]")
                            , pnet::type::value::read ("Struct [b := 0]")
                            )
    );
  require_ctor_exception
    ( "Struct [a:=0] == Struct [a:=0L]"
    , pnet::exception::eval (expr::token::type::eq, 0, 0L)
    );
}

namespace
{
  template<typename T> struct suffix {};

#define SUFFIX(_type, _suffix)                  \
  template<>                                    \
  struct suffix<_type>                          \
  {                                             \
    std::string operator() ()                   \
    {                                           \
      return _suffix;                           \
    }                                           \
  }

  SUFFIX (int, "");
  SUFFIX (unsigned int, "U");
  SUFFIX (long, "L");
  SUFFIX (unsigned long, "UL");
  SUFFIX (float, "f");
  SUFFIX (double, "");
#undef SUFFIX

  template<typename T>
  void require_random_integrals_evaluating_to
    (std::function<void (T const&, T const&)> check)
  {
    std::random_device generator;
    std::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 100; ++i)
    {
      check (number (generator), number (generator));
    }
  }

  template<typename T>
  void check_binop ( std::string const& operation_string
                   , std::function<T (T const&, T const&)> operation
                   , T const& l
                   , T const& r
                   )
  {
    require_evaluating_to
      ( ( boost::format ("%1%%3% %4% %2%%3%")
        % l
        % r
        % suffix<T>()()
        % operation_string
        ).str()
      , operation (l, r)
      );
  }

  template<typename T>
  void check_binop_prefix ( std::string const& operation_string
                          , std::function<T (T const&, T const&)> operation
                          , T const& l
                          , T const& r
                          )
  {
    require_evaluating_to
      ( ( boost::format ("%4% (%1%%3%, %2%%3%)")
        % l
        % r
        % suffix<T>()()
        % operation_string
        ).str()
      , operation (l, r)
      );
  }

  template<typename T>
    T parse_showed (T const& x)
  {
    expr::eval::context context;

    return boost::get<T>
      ( expr::parse::parser
        ((boost::format ("%1%%2%") % x % suffix<T>()()).str())
      . eval_front (context)
      );
  }

  template<typename T>
  void require_random_fractionals_evaluating_to
    (std::function<void (T const&, T const&)> check)
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      check ( parse_showed (number (generator))
            , parse_showed (number (generator))
            );
    }
  }

  template<typename T>
    T plus (T const& l, T const& r)
  {
    return l + r;
  }
}

BOOST_AUTO_TEST_CASE (token_add)
{
  require_evaluating_to ("'a' + 'a'", std::string ("aa"));
  require_evaluating_to ("\"\" + \"\"", std::string (""));
  require_evaluating_to ("\"a\" + \"\"", std::string ("a"));
  require_evaluating_to ("\"a\" + \"a\"", std::string ("aa"));
  require_evaluating_to ("\"ab\" + \"a\"", std::string ("aba"));

  require_random_integrals_evaluating_to<int>
    (std::bind (&check_binop<int>, "+", &plus<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned int>
    (std::bind (&check_binop<unsigned int>, "+", &plus<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_binop<long>, "+", &plus<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned long>
    (std::bind (&check_binop<unsigned long>, "+", &plus<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0 + 0", 0);
  require_evaluating_to ("0 + 1", 1);
  require_evaluating_to ("1 + 0", 1);
  require_evaluating_to ("0U + 0U", 0U);
  require_evaluating_to ("0U + 1U", 1U);
  require_evaluating_to ("1U + 0U", 1U);
  require_evaluating_to ("0L + 0L", 0L);
  require_evaluating_to ("0L + 1L", 1L);
  require_evaluating_to ("1L + 0L", 1L);
  require_evaluating_to ("0UL + 0UL", 0UL);
  require_evaluating_to ("0UL + 1UL", 1UL);
  require_evaluating_to ("1UL + 0UL", 1UL);

  require_random_fractionals_evaluating_to<float>
    (std::bind (&check_binop<float>, "+", &plus<float>, std::placeholders::_1, std::placeholders::_2));
  require_random_fractionals_evaluating_to<double>
    (std::bind (&check_binop<double>, "+", &plus<double>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0.0 + 0.0", 0.0);
  require_evaluating_to ("0.0 + 1.0", 1.0);
  require_evaluating_to ("1.0 + 0.0", 1.0);
  require_evaluating_to ("1.0 + 1.0", 2.0);
  require_evaluating_to ("0.0f + 0.0f", 0.0f);
  require_evaluating_to ("0.0f + 1.0f", 1.0f);
  require_evaluating_to ("1.0f + 0.0f", 1.0f);
  require_evaluating_to ("1.0f + 1.0f", 2.0f);
}

namespace
{
  template<typename T>
    T product (T const& l, T const& r)
  {
    return l * r;
  }
}

BOOST_AUTO_TEST_CASE (token_mul)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_binop<int>, "*", &product<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned int>
    (std::bind (&check_binop<unsigned int>, "*", &product<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_binop<long>, "*", &product<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned long>
    (std::bind (&check_binop<unsigned long>, "*", &product<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0 * 0", 0);
  require_evaluating_to ("0 * 1", 0);
  require_evaluating_to ("1 * 0", 0);
  require_evaluating_to ("1 * 1", 1);
  require_evaluating_to ("1 * 2", 2);
  require_evaluating_to ("2 * 1", 2);
  require_evaluating_to ("0U * 0U", 0U);
  require_evaluating_to ("0U * 1U", 0U);
  require_evaluating_to ("1U * 0U", 0U);
  require_evaluating_to ("1U * 1U", 1U);
  require_evaluating_to ("1U * 2U", 2U);
  require_evaluating_to ("2U * 1U", 2U);
  require_evaluating_to ("0L * 0L", 0L);
  require_evaluating_to ("0L * 1L", 0L);
  require_evaluating_to ("1L * 0L", 0L);
  require_evaluating_to ("1L * 1L", 1L);
  require_evaluating_to ("1L * 2L", 2L);
  require_evaluating_to ("2L * 1L", 2L);
  require_evaluating_to ("0UL * 0UL", 0UL);
  require_evaluating_to ("0UL * 1UL", 0UL);
  require_evaluating_to ("1UL * 0UL", 0UL);
  require_evaluating_to ("1UL * 1UL", 1UL);
  require_evaluating_to ("1UL * 2UL", 2UL);
  require_evaluating_to ("2UL * 1UL", 2UL);

  require_random_fractionals_evaluating_to<float>
    (std::bind (&check_binop<float>, "*", &product<float>, std::placeholders::_1, std::placeholders::_2));
  require_random_fractionals_evaluating_to<double>
    (std::bind (&check_binop<double>, "*", &product<double>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0.0 * 0.0", 0.0);
  require_evaluating_to ("0.0 * 1.0", 0.0);
  require_evaluating_to ("1.0 * 0.0", 0.0);
  require_evaluating_to ("1.0 * 1.0", 1.0);
  require_evaluating_to ("1.0 * 2.0", 2.0);
  require_evaluating_to ("2.0 * 1.0", 2.0);
  require_evaluating_to ("0.0f * 0.0f", 0.0f);
  require_evaluating_to ("0.0f * 1.0f", 0.0f);
  require_evaluating_to ("1.0f * 0.0f", 0.0f);
  require_evaluating_to ("1.0f * 1.0f", 1.0f);
  require_evaluating_to ("1.0f * 2.0f", 2.0f);
  require_evaluating_to ("2.0f * 1.0f", 2.0f);
}

namespace
{
  template<typename T>
    T minimum (T const& l, T const& r)
  {
    return std::min (l, r);
  }

  template<typename T>
    T maximum (T const& l, T const& r)
  {
    return std::max (l, r);
  }
}

BOOST_AUTO_TEST_CASE (token_min)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_binop_prefix<int>, "min", &minimum<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned int>
    (std::bind (&check_binop_prefix<unsigned int>, "min", &minimum<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_binop_prefix<long>, "min", &minimum<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned long>
    (std::bind (&check_binop_prefix<unsigned long>, "min", &minimum<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("min (0, 0)", 0);
  require_evaluating_to ("min (0, 1)", 0);
  require_evaluating_to ("min (1, 0)", 0);
  require_evaluating_to ("min (1, 1)", 1);
  require_evaluating_to ("min (1, 2)", 1);
  require_evaluating_to ("min (2, 1)", 1);
  require_evaluating_to ("min (0U, 0U)", 0U);
  require_evaluating_to ("min (0U, 1U)", 0U);
  require_evaluating_to ("min (1U, 0U)", 0U);
  require_evaluating_to ("min (1U, 1U)", 1U);
  require_evaluating_to ("min (1U, 2U)", 1U);
  require_evaluating_to ("min (2U, 1U)", 1U);
  require_evaluating_to ("min (0L, 0L)", 0L);
  require_evaluating_to ("min (0L, 1L)", 0L);
  require_evaluating_to ("min (1L, 0L)", 0L);
  require_evaluating_to ("min (1L, 1L)", 1L);
  require_evaluating_to ("min (1L, 2L)", 1L);
  require_evaluating_to ("min (2L, 1L)", 1L);
  require_evaluating_to ("min (0UL, 0UL)", 0UL);
  require_evaluating_to ("min (0UL, 1UL)", 0UL);
  require_evaluating_to ("min (1UL, 0UL)", 0UL);
  require_evaluating_to ("min (1UL, 1UL)", 1UL);
  require_evaluating_to ("min (1UL, 2UL)", 1UL);
  require_evaluating_to ("min (2UL, 1UL)", 1UL);

  require_random_fractionals_evaluating_to<float>
    (std::bind (&check_binop_prefix<float>, "min", &minimum<float>, std::placeholders::_1, std::placeholders::_2));
  require_random_fractionals_evaluating_to<double>
    (std::bind (&check_binop_prefix<double>, "min", &minimum<double>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("min (0.0, 0.0)", 0.0);
  require_evaluating_to ("min (0.0, 1.0)", 0.0);
  require_evaluating_to ("min (1.0, 0.0)", 0.0);
  require_evaluating_to ("min (1.0, 1.0)", 1.0);
  require_evaluating_to ("min (1.0, 2.0)", 1.0);
  require_evaluating_to ("min (2.0, 1.0)", 1.0);
  require_evaluating_to ("min (0.0f, 0.0f)", 0.0f);
  require_evaluating_to ("min (0.0f, 1.0f)", 0.0f);
  require_evaluating_to ("min (1.0f, 0.0f)", 0.0f);
  require_evaluating_to ("min (1.0f, 1.0f)", 1.0f);
  require_evaluating_to ("min (1.0f, 2.0f)", 1.0f);
  require_evaluating_to ("min (2.0f, 1.0f)", 1.0f);
}

BOOST_AUTO_TEST_CASE (token_max)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_binop_prefix<int>, "max", &maximum<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned int>
    (std::bind (&check_binop_prefix<unsigned int>, "max", &maximum<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_binop_prefix<long>, "max", &maximum<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned long>
    (std::bind (&check_binop_prefix<unsigned long>, "max", &maximum<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("max (0, 0)", 0);
  require_evaluating_to ("max (0, 1)", 1);
  require_evaluating_to ("max (1, 0)", 1);
  require_evaluating_to ("max (1, 1)", 1);
  require_evaluating_to ("max (1, 2)", 2);
  require_evaluating_to ("max (2, 1)", 2);
  require_evaluating_to ("max (0U, 0U)", 0U);
  require_evaluating_to ("max (0U, 1U)", 1U);
  require_evaluating_to ("max (1U, 0U)", 1U);
  require_evaluating_to ("max (1U, 1U)", 1U);
  require_evaluating_to ("max (1U, 2U)", 2U);
  require_evaluating_to ("max (2U, 1U)", 2U);
  require_evaluating_to ("max (0L, 0L)", 0L);
  require_evaluating_to ("max (0L, 1L)", 1L);
  require_evaluating_to ("max (1L, 0L)", 1L);
  require_evaluating_to ("max (1L, 1L)", 1L);
  require_evaluating_to ("max (1L, 2L)", 2L);
  require_evaluating_to ("max (2L, 1L)", 2L);
  require_evaluating_to ("max (0UL, 0UL)", 0UL);
  require_evaluating_to ("max (0UL, 1UL)", 1UL);
  require_evaluating_to ("max (1UL, 0UL)", 1UL);
  require_evaluating_to ("max (1UL, 1UL)", 1UL);
  require_evaluating_to ("max (1UL, 2UL)", 2UL);
  require_evaluating_to ("max (2UL, 1UL)", 2UL);

  require_random_fractionals_evaluating_to<float>
    (std::bind (&check_binop_prefix<float>, "max", &maximum<float>, std::placeholders::_1, std::placeholders::_2));
  require_random_fractionals_evaluating_to<double>
    (std::bind (&check_binop_prefix<double>, "max", &maximum<double>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("max (0.0, 0.0)", 0.0);
  require_evaluating_to ("max (0.0, 1.0)", 1.0);
  require_evaluating_to ("max (1.0, 0.0)", 1.0);
  require_evaluating_to ("max (1.0, 1.0)", 1.0);
  require_evaluating_to ("max (1.0, 2.0)", 2.0);
  require_evaluating_to ("max (2.0, 1.0)", 2.0);
  require_evaluating_to ("max (0.0f, 0.0f)", 0.0f);
  require_evaluating_to ("max (0.0f, 1.0f)", 1.0f);
  require_evaluating_to ("max (1.0f, 0.0f)", 1.0f);
  require_evaluating_to ("max (1.0f, 1.0f)", 1.0f);
  require_evaluating_to ("max (1.0f, 2.0f)", 2.0f);
  require_evaluating_to ("max (2.0f, 1.0f)", 2.0f);
}

namespace
{
  template<typename T>
    T minus (T const& l, T const& r)
  {
    return l - r;
  }
}

namespace
{
  template<typename T>
  void check_minus_for_unsigned_integral (T const& l, T const& r)
  {
    std::string const expression
      ( ( boost::format ("%1%%3% - %2%%3%")
        % l
        % r
        % suffix<T>()()
        ).str()
      );

    if (l >= r)
    {
      require_evaluating_to (expression, l - r);
    }
    else
    {
      require_ctor_exception
        (expression, std::runtime_error ("r > l => neg result"));
    }
  }
}

BOOST_AUTO_TEST_CASE (token_sub)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_binop<int>, "-", &minus<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_binop<long>, "-", &minus<long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0 - 0", 0);
  require_evaluating_to ("1 - 0", 1);
  require_evaluating_to ("0 - 1", -1);
  require_evaluating_to ("0L - 0L", 0L);
  require_evaluating_to ("1L - 0L", 1L);
  require_evaluating_to ("0L - 1L", -1L);

  require_random_fractionals_evaluating_to<float>
    (std::bind (&check_binop<float>, "-", &minus<float>, std::placeholders::_1, std::placeholders::_2));
  require_random_fractionals_evaluating_to<double>
    (std::bind (&check_binop<double>, "-", &minus<double>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0.0 - 0.0", 0.0);
  require_evaluating_to ("0.0 - 1.0", -1.0);
  require_evaluating_to ("1.0 - 1.0", 0.0);
  require_evaluating_to ("0.0f - 0.0f", 0.0f);
  require_evaluating_to ("0.0f - 1.0f", -1.0f);
  require_evaluating_to ("1.0f - 1.0f", 0.0f);

  require_random_integrals_evaluating_to<unsigned int>
    (&check_minus_for_unsigned_integral<unsigned int>);
  require_random_integrals_evaluating_to<unsigned int>
    (&check_minus_for_unsigned_integral<unsigned long>);

  require_evaluating_to ("0U - 0U", 0U);
  require_evaluating_to ("1U - 0U", 1U);
  require_evaluating_to ("2U - 1U", 1U);
  require_evaluating_to ("0UL - 0UL", 0UL);
  require_evaluating_to ("1UL - 0UL", 1UL);
  require_evaluating_to ("2UL - 1UL", 1UL);

  require_ctor_exception
    ("0U - 1U", std::runtime_error ("r > l => neg result"));
  require_ctor_exception
    ("0UL - 1UL", std::runtime_error ("r > l => neg result"));
}

namespace
{
  template<typename T>
    T quotient (T const& l, T const& r)
  {
    return l / r;
  }

  template<typename T>
  void check_divmod_for_integral
    ( std::string const& operation_string
    , std::function<T (T const&, T const&)> operation
    , T const& l
    , T const& r
    )
  {
    std::string const expression
      ( ( boost::format ("%1%%3% %4% %2%%3%")
        % l
        % r
        % suffix<T>()()
        % operation_string
        ).str()
      );

    if (r != 0)
    {
      require_evaluating_to (expression, operation (l, r));
    }
    else
    {
      require_ctor_exception
        (expression, expr::exception::eval::divide_by_zero());
    }
  }
}

BOOST_AUTO_TEST_CASE (token_divint)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_divmod_for_integral<int>, "div", &quotient<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_divmod_for_integral<unsigned int>, "div", &quotient<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_divmod_for_integral<long>, "div", &quotient<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_divmod_for_integral<unsigned long>, "div", &quotient<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0 div 1", 0);
  require_evaluating_to ("0U div 1U", 0U);
  require_evaluating_to ("0L div 1L", 0L);
  require_evaluating_to ("0UL div 1UL", 0UL);
  require_evaluating_to ("1 div 1", 1);
  require_evaluating_to ("1U div 1U", 1U);
  require_evaluating_to ("1L div 1L", 1L);
  require_evaluating_to ("1UL div 1UL", 1UL);
  require_evaluating_to ("2 div 2", 1);
  require_evaluating_to ("2U div 2U", 1U);
  require_evaluating_to ("2L div 2L", 1L);
  require_evaluating_to ("2UL div 2UL", 1UL);
  require_evaluating_to ("2 div 1", 2);
  require_evaluating_to ("2U div 1U", 2U);
  require_evaluating_to ("2L div 1L", 2L);
  require_evaluating_to ("2UL div 1UL", 2UL);

  require_ctor_exception
    ("1 div 0", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1U div 0U", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1L div 0L", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1UL div 0UL", expr::exception::eval::divide_by_zero());
}

namespace
{
  template<typename T>
    T remainder (T const& l, T const& r)
  {
    return l % r;
  }
}

BOOST_AUTO_TEST_CASE (token_modint)
{
  require_random_integrals_evaluating_to<int>
    (std::bind (&check_divmod_for_integral<int>, "mod", &remainder<int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned int>
    (std::bind (&check_divmod_for_integral<unsigned int>, "mod", &remainder<unsigned int>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<long>
    (std::bind (&check_divmod_for_integral<long>, "mod", &remainder<long>, std::placeholders::_1, std::placeholders::_2));
  require_random_integrals_evaluating_to<unsigned long>
    (std::bind (&check_divmod_for_integral<unsigned long>, "mod", &remainder<unsigned long>, std::placeholders::_1, std::placeholders::_2));

  require_evaluating_to ("0 mod 1", 0);
  require_evaluating_to ("0U mod 1U", 0U);
  require_evaluating_to ("0L mod 1L", 0L);
  require_evaluating_to ("0UL mod 1UL", 0UL);
  require_evaluating_to ("1 mod 1", 0);
  require_evaluating_to ("1U mod 1U", 0U);
  require_evaluating_to ("1L mod 1L", 0L);
  require_evaluating_to ("1UL mod 1UL", 0UL);
  require_evaluating_to ("2 mod 2", 0);
  require_evaluating_to ("2U mod 2U", 0U);
  require_evaluating_to ("2L mod 2L", 0L);
  require_evaluating_to ("2UL mod 2UL", 0UL);
  require_evaluating_to ("2 mod 1", 0);
  require_evaluating_to ("2U mod 1U", 0U);
  require_evaluating_to ("2L mod 1L", 0L);
  require_evaluating_to ("2UL mod 1UL", 0UL);
  require_evaluating_to ("1 mod 2", 1);
  require_evaluating_to ("1U mod 2U", 1U);
  require_evaluating_to ("1L mod 2L", 1L);
  require_evaluating_to ("1UL mod 2UL", 1UL);
  require_evaluating_to ("5 mod 3", 2);
  require_evaluating_to ("5U mod 3U", 2U);
  require_evaluating_to ("5L mod 3L", 2L);
  require_evaluating_to ("5UL mod 3UL", 2UL);

  require_ctor_exception
    ("1 mod 0", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1U mod 0U", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1L mod 0L", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1UL mod 0UL", expr::exception::eval::divide_by_zero());
}

namespace
{
  template<typename T>
  void check_quotient_for_fractional()
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      std::string const l
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());
      std::string const r
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      std::string const expression
        ((boost::format ("%1% / %2%") % l % r).str());

      expr::eval::context context;

      T const r_value
        (boost::get<T> (expr::parse::parser (r).eval_front (context)));

      if (std::abs (r_value) >= std::numeric_limits<T>::min())
      {
        BOOST_REQUIRE_EQUAL
          ( boost::get<T>
            (expr::parse::parser (expression). eval_front (context))
          , boost::get<T> (expr::parse::parser (l).eval_front (context))
          / r_value
          );
      }
      else
      {
        require_ctor_exception
          (expression, expr::exception::eval::divide_by_zero());
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (token_div)
{
  check_quotient_for_fractional<float>();
  check_quotient_for_fractional<double>();

  require_evaluating_to ("0.0 / 1.0", 0.0);
  require_evaluating_to ("0.0f / 1.0f", 0.0f);
  require_evaluating_to ("1.0 / 1.0", 1.0);
  require_evaluating_to ("1.0f / 1.0f", 1.0f);
  require_evaluating_to ("2.0 / 1.0", 2.0);
  require_evaluating_to ("2.0f / 1.0f", 2.0f);
  require_evaluating_to ("2.0 / 2.0", 1.0);
  require_evaluating_to ("2.0f / 2.0f", 1.0f);

  require_ctor_exception
    ("1.0 / 0.0", expr::exception::eval::divide_by_zero());
  require_ctor_exception
    ("1.0f / 0.0f", expr::exception::eval::divide_by_zero());
}

namespace
{
  template<typename T>
    void check_pow_for_fractional()
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      std::string const l
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());
      std::string const r
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      expr::eval::context context;

      BOOST_REQUIRE_EQUAL
        ( boost::get<T>
          ( expr::parse::parser
            ((boost::format ("%1% ** %2%") % l % r).str()).eval_front (context)
          )
        , std::pow
          ( boost::get<T> (expr::parse::parser (l).eval_front (context))
          , boost::get<T> (expr::parse::parser (r).eval_front (context))
          )
        );
    }
  }
}

BOOST_AUTO_TEST_CASE (token_pow)
{
  check_pow_for_fractional<float>();
  check_pow_for_fractional<double>();

  require_evaluating_to ("1.0 ** 0.0", 1.0);
  require_evaluating_to ("1.0f ** 0.0f", 1.0f);
  require_evaluating_to ("1.0 ** 1.0", 1.0);
  require_evaluating_to ("1.0f ** 1.0f", 1.0f);
  require_evaluating_to ("1.0f ** 2.0f", 1.0f);
  require_evaluating_to ("2.0f ** 0.0f", 1.0f);
  require_evaluating_to ("2.0f ** 1.0f", 2.0f);
  require_evaluating_to ("2.0f ** 2.0f", 4.0f);
}

BOOST_AUTO_TEST_CASE (token_pow_int_signed_negative_exponent_throws)
{
  fhg::util::testing::require_exception
    ( [] { require_evaluating_to ("0 ^ (-1)", 0); }
    , expr::exception::eval::negative_exponent()
    );
  fhg::util::testing::require_exception
    ( [] { require_evaluating_to ("0L ^ (-1L)", 0L); }
    , expr::exception::eval::negative_exponent()
    );
}

namespace
{
  template<typename T, typename R>
    void check_unary_for_fractional ( std::string const operation_string
                                    , std::function <R (const T&)> operation
                                    )
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      std::string const input
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      expr::eval::context context;

      BOOST_REQUIRE_EQUAL
        ( boost::get<R>
          ( expr::parse::parser
            ((boost::format ("%1% (%2%)") % operation_string  % input).str())
          .eval_front (context)
          )
        , operation
          (boost::get<T> (expr::parse::parser (input).eval_front (context)))
        );
    }
  }

  template<typename T, typename R>
    void check_unary_for_integral ( std::string const& operation_string
                                  , std::function<R (T const&)> operation
                                  )
  {
    std::random_device generator;
    std::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 100; ++i)
    {
      T const x (number (generator));

      require_evaluating_to
        (( boost::format ("%1% (%2%%3%)")
         % operation_string % x % suffix<T>()()
         ).str()
        , operation (x)
        );
    }
  }

  template<typename T>
    T negate (T const& x)
  {
    return -x;
  }
}

BOOST_AUTO_TEST_CASE (token_neg)
{
  check_unary_for_fractional<float, float> ("-", &negate<float>);
  check_unary_for_fractional<double, double> ("-", &negate<double>);
  check_unary_for_integral<int, int> ("-", &negate<int>);
  check_unary_for_integral<long, long> ("-", &negate<long>);
}

namespace
{
  template<typename T>
    T absolute (T const& x)
  {
    return std::abs (x);
  }
}

BOOST_AUTO_TEST_CASE (token_abs)
{
  check_unary_for_fractional<float, float> ("abs", &absolute<float>);
  check_unary_for_fractional<double, double> ("abs", &absolute<double>);
  check_unary_for_integral<int, int> ("abs", &absolute<int>);
  check_unary_for_integral<long, long> ("abs", &absolute<long>);
}

namespace
{
  template<typename T, typename R>
    R sinus (T const& x)
  {
    return std::sin (x);
  }
}

BOOST_AUTO_TEST_CASE (token_sin)
{
  check_unary_for_fractional<float, float> ("sin", &sinus<float, float>);
  check_unary_for_fractional<double, double> ("sin", &sinus<double, double>);
  check_unary_for_integral<int, double> ("sin", &sinus<int, double>);
  check_unary_for_integral<unsigned int, double> ("sin", &sinus<unsigned int, double>);
  check_unary_for_integral<long, double> ("sin", &sinus<long, double>);
  check_unary_for_integral<unsigned long, double> ("sin", &sinus<unsigned long, double>);
}

namespace
{
  template<typename T, typename R>
    R cosinus (T const& x)
  {
    return std::cos (x);
  }
}

BOOST_AUTO_TEST_CASE (token_cos)
{
  check_unary_for_fractional<float, float> ("cos", &cosinus<float, float>);
  check_unary_for_fractional<double, double> ("cos", &cosinus<double, double>);
  check_unary_for_integral<int, double> ("cos", &cosinus<int, double>);
  check_unary_for_integral<unsigned int, double> ("cos", &cosinus<unsigned int, double>);
  check_unary_for_integral<long, double> ("cos", &cosinus<long, double>);
  check_unary_for_integral<unsigned long, double> ("cos", &cosinus<unsigned long, double>);
}

namespace
{
  template<typename T>
    T floor_fractional (T const& x)
  {
    return std::floor (x);
  }
  template<typename T>
    T floor_integral (T const& x)
  {
    return x;
  }
}

BOOST_AUTO_TEST_CASE (token_floor)
{
  check_unary_for_fractional<float, float> ("floor", &floor_fractional<float>);
  check_unary_for_fractional<double, double> ("floor", &floor_fractional<double>);
  check_unary_for_integral<int, int> ("floor", &floor_integral<int>);
  check_unary_for_integral<unsigned int, unsigned int> ("floor", &floor_integral<unsigned int>);
  check_unary_for_integral<long, long> ("floor", &floor_integral<long>);
  check_unary_for_integral<unsigned long, unsigned long> ("floor", &floor_integral<unsigned long>);
}

namespace
{
  template<typename T>
    T ceil_fractional (T const& x)
  {
    return std::ceil (x);
  }
  template<typename T>
    T ceil_integral (T const& x)
  {
    return x;
  }
}

BOOST_AUTO_TEST_CASE (token_ceil)
{
  check_unary_for_fractional<float, float> ("ceil", &ceil_fractional<float>);
  check_unary_for_fractional<double, double> ("ceil", &ceil_fractional<double>);
  check_unary_for_integral<int, int> ("ceil", &ceil_integral<int>);
  check_unary_for_integral<unsigned int, unsigned int> ("ceil", &ceil_integral<unsigned int>);
  check_unary_for_integral<long, long> ("ceil", &ceil_integral<long>);
  check_unary_for_integral<unsigned long, unsigned long> ("ceil", &ceil_integral<unsigned long>);
}

namespace
{
  template<typename T>
    T round_fractional (T const& x)
  {
    return std::round (x);
  }
  template<typename T>
    T round_integral (T const& x)
  {
    return x;
  }
}

BOOST_AUTO_TEST_CASE (token_round)
{
  check_unary_for_fractional<float, float> ("round", &round_fractional<float>);
  check_unary_for_fractional<double, double> ("round", &round_fractional<double>);
  check_unary_for_integral<int, int> ("round", &round_integral<int>);
  check_unary_for_integral<unsigned int, unsigned int> ("round", &round_integral<unsigned int>);
  check_unary_for_integral<long, long> ("round", &round_integral<long>);
  check_unary_for_integral<unsigned long, unsigned long> ("round", &round_integral<unsigned long>);
}

namespace
{
  template<typename T, typename R>
    R cast (T const& x)
  {
    return static_cast<R> (x);
  }

  template<typename T>
  void check_token_cast (std::string const& tag)
  {
    check_unary_for_fractional<float, T> (tag, &cast<float, T>);
    check_unary_for_fractional<double, T> (tag, &cast<double, T>);
    check_unary_for_integral<int, T> (tag, &cast<int, T>);
    check_unary_for_integral<unsigned int, T> (tag, &cast<unsigned int, T>);
    check_unary_for_integral<long, T> (tag, &cast<long, T>);
    check_unary_for_integral<unsigned long, T> (tag, &cast<unsigned long, T>);
  }
}

BOOST_AUTO_TEST_CASE (tokens_cast)
{
  check_token_cast<int> ("int");
  check_token_cast<unsigned int> ("uint");
  check_token_cast<long> ("long");
  check_token_cast<unsigned long> ("ulong");
  check_token_cast<float> ("float");
  check_token_cast<double> ("double");

  require_evaluating_to ("int (true)", 1);
  require_evaluating_to ("int (false)", 0);
  require_evaluating_to ("uint (true)", 1U);
  require_evaluating_to ("uint (false)", 0U);
  require_evaluating_to ("long (true)", 1L);
  require_evaluating_to ("long (false)", 0L);
  require_evaluating_to ("ulong (true)", 1UL);
  require_evaluating_to ("ulong (false)", 0UL);
}

namespace
{
  template<typename T>
  void check_square_root_for_fractional()
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      std::string const input
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      expr::eval::context _;

      std::string const expression
        ((boost::format ("sqrt (%1%)") % input).str());

      T const value
        (boost::get<T> (expr::parse::parser (input).eval_front (_)));

      if (value < 0)
      {
        require_ctor_exception
          ( expression
          , expr::exception::eval::square_root_for_negative_argument<T> (value)
          );
      }
      else
      {
        BOOST_REQUIRE_EQUAL
          ( boost::get<T> (expr::parse::parser (expression).eval_front (_))
          , std::sqrt (value)
          );
      }
    }
  }

  template<typename T>
  void check_square_root_for_signed_integral()
  {
    std::random_device generator;
    std::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 100; ++i)
    {
      T const x (number (generator));

      std::string const expression
        ((boost::format ("sqrt (%1%%2%)") % x % suffix<T>()()).str());

      if (x < 0)
      {
        require_ctor_exception
          ( expression
          , expr::exception::eval::square_root_for_negative_argument<T> (x)
          );
      }
      else
      {
        require_evaluating_to (expression, std::sqrt (x));
      }
    }
  }

  template<typename T>
    double square_root (T const& x)
  {
    return std::sqrt (x);
  }
}

BOOST_AUTO_TEST_CASE (token_sqrt)
{
  check_square_root_for_fractional<float>();
  check_square_root_for_fractional<double>();
  check_unary_for_integral<unsigned int, double> ("sqrt", &square_root<unsigned int>);
  check_unary_for_integral<unsigned long, double> ("sqrt", &square_root<unsigned long>);
  check_square_root_for_signed_integral<int>();
  check_square_root_for_signed_integral<long>();
}

namespace
{
  template<typename T>
  void check_logarithm_for_fractional()
  {
    std::random_device generator;
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );

    for (int i (0); i < 100; ++i)
    {
      std::string const input
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      expr::eval::context _;

      std::string const expression
        ((boost::format ("log (%1%)") % input).str());

      T const value
        (boost::get<T> (expr::parse::parser (input).eval_front (_)));

      if (value < 0)
      {
        require_ctor_exception
          ( expression
          , expr::exception::eval::log_for_nonpositive_argument<T> (value)
          );
      }
      else
      {
        BOOST_REQUIRE_EQUAL
          ( boost::get<T> (expr::parse::parser (expression).eval_front (_))
          , std::log (value)
          );
      }
    }
  }

  template<typename T>
  void check_logarithm_for_signed_integral()
  {
    std::random_device generator;
    std::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 100; ++i)
    {
      T const x (number (generator));

      std::string const expression
        ((boost::format ("log (%1%%2%)") % x % suffix<T>()()).str());

      if (!(x > 0))
      {
        require_ctor_exception
          ( expression
          , expr::exception::eval::log_for_nonpositive_argument<T> (x)
          );
      }
      else
      {
        require_evaluating_to (expression, std::log (x));
      }
    }
  }

  template<typename T>
    double logarithm (T const& x)
  {
    return std::log (x);
  }
}

BOOST_AUTO_TEST_CASE (token_log)
{
  check_logarithm_for_fractional<float>();
  check_logarithm_for_fractional<double>();
  check_unary_for_integral<unsigned int, double> ("log", &logarithm<unsigned int>);
  check_unary_for_integral<unsigned long, double> ("log", &logarithm<unsigned long>);
  check_logarithm_for_signed_integral<int>();
  check_logarithm_for_signed_integral<long>();
}

namespace
{
  void check_random_bitsets
    (std::function<void (bitsetofint::type const&)> check)
  {
    std::random_device generator;
    std::uniform_int_distribution<int> count (0, 100);
    std::uniform_int_distribution<unsigned long> number (0, 1UL << 10);

    for (int _ (0); _ < 100; ++_)
    {
      bitsetofint::type bs;

      int n (count (generator));

      while (n --> 0)
      {
        bs.ins (number (generator));
      }

      check (bs);
    }
  }

  void check_random_pairs_of_bitsets
  ( std::function<void ( bitsetofint::type const&
                       , bitsetofint::type const&
                       )
                 > check
  )
  {
    std::random_device generator;
    std::uniform_int_distribution<int> count (0, 100);
    std::uniform_int_distribution<unsigned long> number (0, 1UL << 10);

    for (int _ (0); _ < 100; ++_)
    {
      bitsetofint::type bs_l;

      {
        int n (count (generator));

        while (n --> 0)
        {
          bs_l.ins (number (generator));
        }
      }

      bitsetofint::type bs_r;

      {
        int n (count (generator));

        while (n --> 0)
        {
          bs_r.ins (number (generator));
        }
      }

      check (bs_l, bs_r);
    }
  }
}

BOOST_AUTO_TEST_CASE (token_bitset_count)
{
  check_random_bitsets
    ([](bitsetofint::type const& bs)
    {
      require_evaluating_to
        (boost::format ("bitset_count (%1%)") % bs, bs.count());
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_fromhex_tohex_is_id)
{
  check_random_bitsets
    ([](bitsetofint::type const& bs)
    {
      require_evaluating_to
        (boost::format ("bitset_fromhex (bitset_tohex (%1%))") % bs, bs);
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_logical)
{
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (boost::format ("bitset_or (%1%, %2%)") % l % r, l | r);
    }
    );
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (boost::format ("bitset_and (%1%, %2%)") % l % r, l & r);
    }
    );
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (boost::format ("bitset_xor (%1%, %2%)") % l % r, l ^ r);
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_ins_del_is_elem)
{
  std::random_device generator;
  std::uniform_int_distribution<int> count (0, 100);
  std::uniform_int_distribution<unsigned long> number (0, 1UL << 10);

  for (int _ (0); _ < 100; ++_)
  {
    std::set<unsigned long> ks;
    bitsetofint::type a;
    bitsetofint::type b;

    int n (count (generator));

    while (n --> 0)
    {
      unsigned long const k (number (generator));

      ks.insert (k);

      require_evaluating_to
        ( boost::format ("bitset_is_element (%1%, %2%UL)") % a % k
        , b.is_element (k)
        );

      b.ins (k);

      require_evaluating_to
        (boost::format ("bitset_insert (%1%, %2%UL)") % a % k, b);

      a.ins (k);

      require_evaluating_to
        (boost::format ("bitset_is_element (%1%, %2%UL)") % a % k, true);
    }

    for (unsigned long k : ks)
    {
      require_evaluating_to
        (boost::format ("bitset_is_element (%1%, %2%UL)") % a % k, true);

      b.del (k);

      require_evaluating_to
        (boost::format ("bitset_delete (%1%, %2%UL)") % a % k, b);

      a.del (k);

      require_evaluating_to
        (boost::format ("bitset_is_element (%1%, %2%UL)") % a % k, false);
    }
  }
}

BOOST_AUTO_TEST_CASE (tokens_stack_push_top_pop_empty_size)
{
  std::random_device generator;
  std::uniform_int_distribution<unsigned long> count (0, 100);
  std::uniform_int_distribution<int> number
    (std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

  for (int _ (0); _ < 10; ++_)
  {
    std::list<pnet::type::value::value_type> a;
    std::list<pnet::type::value::value_type> b;

    require_evaluating_to
      ( ( boost::format ("stack_empty (%1%)")
        % pnet::type::value::show (a)
        ).str()
      , true
      );

    unsigned long n (count (generator));

    std::stack<int> ks;

    for (unsigned long i (0); i < n; ++i)
    {
      require_evaluating_to
        (boost::format ("stack_size (%1%)") % pnet::type::value::show (a), i);

      int const k (number (generator));

      ks.push (k);

      b.push_back (k);

      require_evaluating_to
        ( boost::format ("stack_push (%1%, %2%)")
        % pnet::type::value::show (a) % k
        , b
        );

      a.push_back (k);

      require_evaluating_to
        (boost::format ("stack_top (%1%)") % pnet::type::value::show (a), k);

      require_evaluating_to
        ( ( boost::format ("stack_empty (%1%)")
          % pnet::type::value::show (a)
          ).str()
        , false
        );
    }

    while (!ks.empty())
    {
      require_evaluating_to
        ( boost::format ("stack_size (%1%)") % pnet::type::value::show (a)
        , ks.size()
        );

      require_evaluating_to
        ( boost::format ("stack_top (%1%)") % pnet::type::value::show (a)
        , ks.top()
        );

      b.pop_back();

      require_evaluating_to
        (boost::format ("stack_pop (%1%)") % pnet::type::value::show (a), b);

      a.pop_back();

      ks.pop();
    }

    require_evaluating_to
      ( ( boost::format ("stack_empty (%1%)")
        % pnet::type::value::show (a)
        ).str()
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (token_stack_join)
{
  std::random_device generator;
  std::uniform_int_distribution<unsigned long> count (0, 100);
  std::uniform_int_distribution<int> number
    (std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

  for (int _ (0); _ < 10; ++_)
  {
    std::list<pnet::type::value::value_type> a;
    std::list<pnet::type::value::value_type> b;
    std::list<pnet::type::value::value_type> joined;

    unsigned long na (count (generator));
    unsigned long nb (count (generator));

    while (na --> 0)
    {
      a.push_back (number (generator));
      joined.push_back (a.back());
    }
    while (nb --> 0)
    {
      b.push_back (number (generator));
      joined.push_back (b.back());
    }

    require_evaluating_to
      ( ( boost::format ("stack_join (%1%, %2%)")
        % pnet::type::value::show (a)
        % pnet::type::value::show (b)
        ).str()
      , joined
      );
  }
}

BOOST_AUTO_TEST_CASE
  (tokens_map_assign_unassign_is_assigned_get_assignment_size_empty)
{
  std::random_device generator;
  std::uniform_int_distribution<int> count (0, 100);
  std::uniform_int_distribution<int> key
    (std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
  std::uniform_int_distribution<long> value
    (std::numeric_limits<long>::min(), std::numeric_limits<long>::max());

  for (int _ (0); _ < 10; ++_)
  {
    std::map<pnet::type::value::value_type, pnet::type::value::value_type> a;
    std::map<pnet::type::value::value_type, pnet::type::value::value_type> b;

    require_evaluating_to
      ( (boost::format ("map_empty (%1%)") % pnet::type::value::show (a)).str()
      , true
      );

    int n (count (generator));
    std::set<int> ks;

    while (n --> 0)
    {
      require_evaluating_to
        ( boost::format ("map_size (%1%)") % pnet::type::value::show (a)
        , a.size()
        );

      int const k (key (generator));
      long const v (value (generator));

      ks.insert (k);

      require_evaluating_to
        ( boost::format ("map_is_assigned (%1%, %2%)")
        % pnet::type::value::show (a)
        % k
        , a.find (k) != a.end()
        );

      b.emplace (k, v);

      require_evaluating_to
        ( ( boost::format ("map_assign (%1%, %2%, %3%L)")
          % pnet::type::value::show (a)
          % k
          % v
          ).str()
        , b
        );

      a.emplace (k, v);

      require_evaluating_to
        ( boost::format ("map_is_assigned (%1%, %2%)")
        % pnet::type::value::show (a)
        % k
        , true
        );

      require_evaluating_to
        ( boost::format ("map_get_assignment (%1%, %2%)")
        % pnet::type::value::show (a)
        % k
        , v
        );

      require_evaluating_to
        ( ( boost::format ("map_empty (%1%)") % pnet::type::value::show (a)
          ).str()
        , false
        );
    }

    for (int k : ks)
    {
      require_evaluating_to
        ( boost::format ("map_is_assigned (%1%, %2%)")
        % pnet::type::value::show (a)
        % k
        , true
        );

      require_evaluating_to
        ( boost::format ("map_get_assignment (%1%, %2%)")
        % pnet::type::value::show (a)
        % k
        , a.at (k)
        );

      require_evaluating_to
        ( ( boost::format ("map_empty (%1%)") % pnet::type::value::show (a)
          ).str()
        , false
        );

      b.erase (k);

      require_evaluating_to
        ( ( boost::format ("map_unassign (%1%, %2%)")
          % pnet::type::value::show (a)
          % k
          ).str()
        , b
        );

      a.erase (k);
    }

    require_evaluating_to
      ( (boost::format ("map_empty (%1%)") % pnet::type::value::show (a)).str()
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (tokens_set_empty_size_insert_erase_is_element)
{
  std::random_device generator;
  std::uniform_int_distribution<int> count (0, 100);
  std::uniform_int_distribution<long> number
    (std::numeric_limits<long>::min(), std::numeric_limits<long>::max());

  for (int _ (0); _ < 10; ++_)
  {
    std::set<pnet::type::value::value_type> a;
    std::set<pnet::type::value::value_type> b;

    require_evaluating_to
      ( (boost::format ("set_empty (%1%)") % pnet::type::value::show (a)).str()
      , true
      );

    int n (count (generator));
    std::set<long> ks;

    while (n --> 0)
    {
      require_evaluating_to
        ( boost::format ("set_size (%1%)") % pnet::type::value::show (a)
        , a.size()
        );

      long const k (number (generator));
      ks.insert (k);

      require_evaluating_to
        ( boost::format ("set_is_element (%1%, %2%L)")
        % pnet::type::value::show (a)
        % k
        , a.find (k) != a.end()
        );

      b.insert (k);

      require_evaluating_to
        ( boost::format ("set_insert (%1%, %2%L)")
        % pnet::type::value::show (a)
        % k
        , b
        );

      a.insert (k);
    }

    for (long k : ks)
    {
      require_evaluating_to
        ( boost::format ("set_size (%1%)") % pnet::type::value::show (a)
        , a.size()
        );

      require_evaluating_to
        ( boost::format ("set_is_element (%1%, %2%L)")
        % pnet::type::value::show (a)
        % k
        , true
        );

      b.erase (k);

      require_evaluating_to
        ( boost::format ("set_erase (%1%, %2%L)")
        % pnet::type::value::show (a)
        % k
        , b
        );

      a.erase (k);
    }

    require_evaluating_to
      ( (boost::format ("set_empty (%1%)") % pnet::type::value::show (a)).str()
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (tokens_set_top_pop)
{
  std::random_device generator;
  std::uniform_int_distribution<int> count (0, 100);
  std::uniform_int_distribution<long> number
    (std::numeric_limits<long>::min(), std::numeric_limits<long>::max());

  for (int _ (0); _ < 10; ++_)
  {
    std::set<pnet::type::value::value_type> a;
    std::set<pnet::type::value::value_type> b;

    int n (count (generator));

    while (n --> 0)
    {
      long const k (number (generator));

      a.insert (k);
      b.insert (k);

      require_evaluating_to
        ( (boost::format ("set_top (%1%)") % pnet::type::value::show (a)).str()
        , *(a.begin())
        );
    }

    while (!a.empty())
    {
      b.erase (b.begin());

      require_evaluating_to
        ( (boost::format ("set_pop (%1%)") % pnet::type::value::show (a)).str()
        , b
        );

      a.erase (a.begin());
    }
  }
}

BOOST_AUTO_TEST_CASE (token_set_is_subset)
{
  require_evaluating_to ("set_is_subset (Set{}, Set{})", true);
  require_evaluating_to
    ("set_is_subset (set_insert (Set{}, 1L), Set{})", false);
  require_evaluating_to
    ("set_is_subset (set_insert (Set{}, 1L), set_insert (Set{}, 1L))", true);
  require_evaluating_to
    ("set_is_subset (Set{}, set_insert (Set{}, 1L))", true);
  require_evaluating_to
    ("set_is_subset (set_insert (Set{}, 1), set_insert (Set{}, 1L))", false);
  require_evaluating_to
    ("set_is_subset (set_insert (Set{}, 1), set_insert (Set{}, 2))", false);
}
