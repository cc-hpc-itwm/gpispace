// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/token/type.hpp>
#include <we/type/value/boost/test/printer.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <FMT/we/type/value/show.hpp>
#include <fmt/core.h>
#include <functional>
#include <iterator>
#include <limits>
#include <random>
#include <stack>
#include <string>

namespace
{
  template<typename T>
  expr::eval::context require_evaluating_to
    (std::string const& expression, T const& value)
  {
    expr::eval::context context;

    if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
      BOOST_TEST
        (  boost::get<T> (expr::parse::parser (expression).eval_all (context))
        == value
        , boost::test_tools::tolerance (1e-6)
        );
    }
    else
    {
      BOOST_REQUIRE_EQUAL ( expr::parse::parser (expression).eval_all (context)
                          , pnet::type::value::value_type (value)
                          );
    }

    return context;
  }
}

BOOST_AUTO_TEST_CASE (empty_expression_evaluates_to_control)
{
  require_evaluating_to ("", we::type::literal::control{});
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
    ( [] { require_evaluating_to ("true || (${a} := true)", true).value ({"a"}); }
    , pnet::exception::missing_binding ("a")
    );

  BOOST_REQUIRE_EQUAL
    ( require_evaluating_to ("false || (${a} := true)", true).value ({"a"})
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
    ( [] { require_evaluating_to ("false && (${a} := false)", false).value ({"a"}); }
    , pnet::exception::missing_binding ("a")
    );

  BOOST_REQUIRE_EQUAL
    ( require_evaluating_to ("true && (${a} := false)", false).value ({"a"})
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
    require_evaluating_to (fmt::format ("{} < {}", lhs, rhs), lt);
    require_evaluating_to (fmt::format ("{} <= {}", lhs, rhs), le);
    require_evaluating_to (fmt::format ("{} > {}", lhs, rhs), gt);
    require_evaluating_to (fmt::format ("{} >= {}", lhs, rhs), ge);

    require_evaluating_to (fmt::format ("{} :lt: {}", lhs, rhs), lt);
    require_evaluating_to (fmt::format ("{} :le: {}", lhs, rhs), le);
    require_evaluating_to (fmt::format ("{} :gt: {}", lhs, rhs), gt);
    require_evaluating_to (fmt::format ("{} :ge: {}", lhs, rhs), ge);
  }

  void check_equality (std::string lhs, std::string rhs, bool eq)
  {
    require_evaluating_to (fmt::format ("{} != {}", lhs, rhs), !eq);
    require_evaluating_to (fmt::format ("{} == {}", lhs, rhs), eq);

    require_evaluating_to (fmt::format ("{} :ne: {}", lhs, rhs), !eq);
    require_evaluating_to (fmt::format ("{} :eq: {}", lhs, rhs), eq);
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
    fhg::util::testing::random<T> number;

    for (int i (0); i < 100; ++i)
    {
      check (number(), number());
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
      ( fmt::format ( "{0}{2} {3} {1}{2}"
                    , l
                    , r
                    , suffix<T>()()
                    , operation_string
                    )
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
      ( fmt::format ( "{3} ({0}{2}, {1}{2})"
                    , l
                    , r
                    , suffix<T>()()
                    , operation_string
                    )
      , operation (l, r)
      );
  }

  template<typename T>
    T parse_showed (T const& x)
  {
    expr::eval::context context;

    return ::boost::get<T>
      ( expr::parse::parser {fmt::format ("{:g}{}", x, suffix<T>()())}
      . eval_all (context)
      );
  }

  template<typename T>
    T random_real()
  {
    std::uniform_real_distribution<T> number
      ( -std::numeric_limits<T>::max() / T (2.0)
      ,  std::numeric_limits<T>::max() / T (2.0)
      );
    return number (fhg::util::testing::detail::GLOBAL_random_engine());
  }

  template<typename T>
  void require_random_fractionals_evaluating_to
    (std::function<void (T const&, T const&)> check)
  {
    for (int i (0); i < 100; ++i)
    {
      check ( parse_showed (random_real<T>())
            , parse_showed (random_real<T>())
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
  require_evaluating_to (R"("" + "")", std::string (""));
  require_evaluating_to (R"("a" + "")", std::string ("a"));
  require_evaluating_to (R"("a" + "a")", std::string ("aa"));
  require_evaluating_to (R"("ab" + "a")", std::string ("aba"));

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
      { fmt::format ( "{0}{2} - {1}{2}"
                    , l
                    , r
                    , suffix<T>()()
                    )
      };

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
      { fmt::format ( "{0}{2} {3} {1}{2}"
                    , l
                    , r
                    , suffix<T>()()
                    , operation_string
                    )
      };

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
    for (int i (0); i < 100; ++i)
    {
      std::string const l
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};
      std::string const r
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};

      std::string const expression {fmt::format ("{} / {}", l, r)};

      expr::eval::context context;

      T const r_value
        (::boost::get<T> (expr::parse::parser (r).eval_all (context)));

      if (std::abs (r_value) >= std::numeric_limits<T>::min())
      {
        BOOST_REQUIRE_EQUAL
          ( ::boost::get<T>
            (expr::parse::parser (expression). eval_all (context))
          , ::boost::get<T> (expr::parse::parser (l).eval_all (context))
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
    for (int i (0); i < 100; ++i)
    {
      std::string const l
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};
      std::string const r
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};

      expr::eval::context context;

      BOOST_REQUIRE_EQUAL
        ( ::boost::get<T>
          ( expr::parse::parser {fmt::format ("{} ** {}", l, r)}
          . eval_all (context)
          )
        , std::pow
          ( ::boost::get<T> (expr::parse::parser (l).eval_all (context))
          , ::boost::get<T> (expr::parse::parser (r).eval_all (context))
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
                                    , std::function <R (T const&)> operation
                                    )
  {
    for (int i (0); i < 100; ++i)
    {
      std::string const input
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};

      expr::eval::context context;

      BOOST_REQUIRE_EQUAL
        ( ::boost::get<R>
          ( expr::parse::parser
              {fmt::format ("{} ({})", operation_string, input)}
          . eval_all (context)
          )
        , operation
          (::boost::get<T> (expr::parse::parser (input).eval_all (context)))
        );
    }
  }

  template<typename T, typename R>
    void check_unary_for_integral ( std::string const& operation_string
                                  , std::function<R (T const&)> operation
                                  )
  {
    fhg::util::testing::random<T> number;

    for (int i (0); i < 100; ++i)
    {
      T const x (number());

      require_evaluating_to
        ( fmt::format ( "{} ({}{})"
                      , operation_string
                      , x
                      , suffix<T>()()
                      )
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

BOOST_AUTO_TEST_CASE (integral_followed_by_f_is_parsed_as_float)
{
  require_evaluating_to ("0f", 0.f);
  require_evaluating_to ("-1f", -1.f);
  require_evaluating_to ("1f", 1.f);
  require_evaluating_to ("42f", 42.f);

  auto random_int (fhg::util::testing::random<int>{});

  for (int i (0); i < 100; ++i)
  {
    auto const x (random_int());

    require_evaluating_to
      (fmt::format ("{}f", x), static_cast<float> (x));
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
    for (int i (0); i < 100; ++i)
    {
      std::string const input
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};

      expr::eval::context _;

      std::string const expression {fmt::format ("sqrt ({})", input)};

      T const value
        (::boost::get<T> (expr::parse::parser (input).eval_all (_)));

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
          ( ::boost::get<T> (expr::parse::parser (expression).eval_all (_))
          , std::sqrt (value)
          );
      }
    }
  }

  template<typename T>
  void check_square_root_for_signed_integral()
  {
    fhg::util::testing::random<T> number;

    for (int i (0); i < 100; ++i)
    {
      T const x (number());

      std::string const expression
        {fmt::format ("sqrt ({}{})", x, suffix<T>()())};

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
    for (int i (0); i < 100; ++i)
    {
      std::string const input
        {fmt::format ("{}{}", random_real<T>(), suffix<T>()())};

      expr::eval::context _;

      std::string const expression {fmt::format ("log ({})", input)};

      T const value
        (::boost::get<T> (expr::parse::parser (input).eval_all (_)));

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
          ( ::boost::get<T> (expr::parse::parser (expression).eval_all (_))
          , std::log (value)
          );
      }
    }
  }

  template<typename T>
  void check_logarithm_for_signed_integral()
  {
    fhg::util::testing::random<T> number;

    for (int i (0); i < 100; ++i)
    {
      T const x (number());

      std::string const expression
        {fmt::format ("log ({}{})", x, suffix<T>()())};

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
    for (int _ (0); _ < 100; ++_)
    {
      bitsetofint::type bs;

      auto n (fhg::util::testing::random<unsigned int>{} (100));

      while (n --> 0)
      {
        bs.ins (fhg::util::testing::random<unsigned long>{} (1UL << 10));
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
    for (int _ (0); _ < 100; ++_)
    {
      bitsetofint::type bs_l;

      {
        auto n (fhg::util::testing::random<unsigned int>{} (100));

        while (n --> 0)
        {
          bs_l.ins (fhg::util::testing::random<unsigned long>{} (1UL << 10));
        }
      }

      bitsetofint::type bs_r;

      {
        auto n (fhg::util::testing::random<unsigned int>{} (100));

        while (n --> 0)
        {
          bs_r.ins (fhg::util::testing::random<unsigned long>{} (1UL << 10));
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
        (fmt::format ("bitset_count ({})", bs), bs.count());
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_fromhex_tohex_is_id)
{
  check_random_bitsets
    ([](bitsetofint::type const& bs)
    {
      require_evaluating_to
        (fmt::format ("bitset_fromhex (bitset_tohex ({}))", bs), bs);
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_logical)
{
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (fmt::format ("bitset_or ({}, {})", l, r), l | r);
    }
    );
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (fmt::format ("bitset_and ({}, {})", l, r), l & r);
    }
    );
  check_random_pairs_of_bitsets
    ([](bitsetofint::type const& l, bitsetofint::type const& r)
    {
      require_evaluating_to
        (fmt::format ("bitset_xor ({}, {})", l, r), l ^ r);
    }
    );
}

BOOST_AUTO_TEST_CASE (token_bitset_ins_del_is_elem)
{
  for (int _ (0); _ < 100; ++_)
  {
    std::set<unsigned long> ks;
    bitsetofint::type a;
    bitsetofint::type b;

    auto n (fhg::util::testing::random<unsigned int>{} (100));

    while (n --> 0)
    {
      auto const k (fhg::util::testing::random<unsigned long>{} (1UL << 10));

      ks.insert (k);

      require_evaluating_to
        ( fmt::format ("bitset_is_element ({}, {}UL)", a, k)
        , b.is_element (k)
        );

      b.ins (k);

      require_evaluating_to
        (fmt::format ("bitset_insert ({}, {}UL)", a, k), b);

      a.ins (k);

      require_evaluating_to
        (fmt::format ("bitset_is_element ({}, {}UL)", a, k), true);
    }

    for (unsigned long k : ks)
    {
      require_evaluating_to
        (fmt::format ("bitset_is_element ({}, {}UL)", a, k), true);

      b.del (k);

      require_evaluating_to
        (fmt::format ("bitset_delete ({}, {}UL)", a, k), b);

      a.del (k);

      require_evaluating_to
        (fmt::format ("bitset_is_element ({}, {}UL)", a, k), false);
    }
  }
}

BOOST_AUTO_TEST_CASE (tokens_stack_push_top_pop_empty_size)
{
  fhg::util::testing::random<int> number;

  for (int _ (0); _ < 10; ++_)
  {
    std::list<pnet::type::value::value_type> a;
    std::list<pnet::type::value::value_type> b;

    require_evaluating_to
      ( fmt::format ("stack_empty ({})", pnet::type::value::show (a))
      , true
      );

    auto const n (fhg::util::testing::random<unsigned long>{} (100));

    std::stack<int> ks;

    for (unsigned long i (0); i < n; ++i)
    {
      require_evaluating_to
        (fmt::format ("stack_size ({})", pnet::type::value::show (a)), i);

      int const k (number());

      ks.push (k);

      b.push_back (k);

      require_evaluating_to
        ( fmt::format ( "stack_push ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , b
        );

      a.push_back (k);

      require_evaluating_to
        (fmt::format ("stack_top ({})", pnet::type::value::show (a)), k);

      require_evaluating_to
        ( fmt::format ("stack_empty ({})", pnet::type::value::show (a))
        , false
        );
    }

    while (!ks.empty())
    {
      require_evaluating_to
        ( fmt::format ("stack_size ({})", pnet::type::value::show (a))
        , ks.size()
        );

      require_evaluating_to
        ( fmt::format ("stack_top ({})", pnet::type::value::show (a))
        , ks.top()
        );

      b.pop_back();

      require_evaluating_to
        (fmt::format ("stack_pop ({})", pnet::type::value::show (a)), b);

      a.pop_back();

      ks.pop();
    }

    require_evaluating_to
      ( fmt::format ("stack_empty ({})", pnet::type::value::show (a))
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (token_stack_join)
{
  fhg::util::testing::random<int> number;

  for (int _ (0); _ < 10; ++_)
  {
    std::list<pnet::type::value::value_type> a;
    std::list<pnet::type::value::value_type> b;
    std::list<pnet::type::value::value_type> joined;

    auto na (fhg::util::testing::random<unsigned long>{} (100));
    auto nb (fhg::util::testing::random<unsigned long>{} (100));

    while (na --> 0)
    {
      a.push_back (number());
      joined.push_back (a.back());
    }
    while (nb --> 0)
    {
      b.push_back (number());
      joined.push_back (b.back());
    }

    require_evaluating_to
      ( fmt::format ( "stack_join ({}, {})"
                    , pnet::type::value::show (a)
                    , pnet::type::value::show (b)
                    )
      , joined
      );
  }
}

BOOST_AUTO_TEST_CASE
  (tokens_map_assign_unassign_is_assigned_get_assignment_size_empty)
{
  fhg::util::testing::random<int> key;
  fhg::util::testing::random<long> value;

  for (int _ (0); _ < 10; ++_)
  {
    std::map<pnet::type::value::value_type, pnet::type::value::value_type> a;
    std::map<pnet::type::value::value_type, pnet::type::value::value_type> b;

    require_evaluating_to
      ( fmt::format ("map_empty ({})", pnet::type::value::show (a))
      , true
      );

    auto n (fhg::util::testing::random<unsigned int>{} (100));
    std::set<int> ks;

    while (n --> 0)
    {
      require_evaluating_to
        ( fmt::format ("map_size ({})", pnet::type::value::show (a))
        , a.size()
        );

      int const k (key());
      long const v (value());

      ks.insert (k);

      require_evaluating_to
        ( fmt::format ( "map_is_assigned ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , a.find (k) != a.end()
        );

      b.emplace (k, v);

      require_evaluating_to
        ( fmt::format ( "map_assign ({}, {}, {}L)"
                      , pnet::type::value::show (a)
                      , k
                      , v
                      )
        , b
        );

      a.emplace (k, v);

      require_evaluating_to
        ( fmt::format ( "map_is_assigned ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , true
        );

      require_evaluating_to
        ( fmt::format ( "map_get_assignment ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , v
        );

      require_evaluating_to
        ( fmt::format ("map_empty ({})", pnet::type::value::show (a))
        , false
        );
    }

    for (int k : ks)
    {
      require_evaluating_to
        ( fmt::format ( "map_is_assigned ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , true
        );

      require_evaluating_to
        ( fmt::format ( "map_get_assignment ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , a.at (k)
        );

      require_evaluating_to
        ( fmt::format ("map_empty ({})", pnet::type::value::show (a))
        , false
        );

      b.erase (k);

      require_evaluating_to
        ( fmt::format ( "map_unassign ({}, {})"
                      , pnet::type::value::show (a)
                      , k
                      )
        , b
        );

      a.erase (k);
    }

    require_evaluating_to
      ( fmt::format ("map_empty ({})", pnet::type::value::show (a))
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (tokens_set_empty_size_insert_erase_is_element)
{
  fhg::util::testing::random<long> number;

  for (int _ (0); _ < 10; ++_)
  {
    std::set<pnet::type::value::value_type> a;
    std::set<pnet::type::value::value_type> b;

    require_evaluating_to
      ( fmt::format ("set_empty ({})", pnet::type::value::show (a))
      , true
      );

    auto n (fhg::util::testing::random<unsigned int>{} (100));
    std::set<long> ks;

    while (n --> 0)
    {
      require_evaluating_to
        ( fmt::format ("set_size ({})",  pnet::type::value::show (a))
        , a.size()
        );

      long const k (number());
      ks.insert (k);

      require_evaluating_to
        ( fmt::format ( "set_is_element ({}, {}L)"
                      , pnet::type::value::show (a)
                      , k
                      )
        , a.find (k) != a.end()
        );

      b.insert (k);

      require_evaluating_to
        ( fmt::format ( "set_insert ({}, {}L)"
                      , pnet::type::value::show (a)
                      , k
                      )
        , b
        );

      a.insert (k);
    }

    for (long k : ks)
    {
      require_evaluating_to
        ( fmt::format ("set_size ({})", pnet::type::value::show (a))
        , a.size()
        );

      require_evaluating_to
        ( fmt::format ( "set_is_element ({}, {}L)"
                      , pnet::type::value::show (a)
                      , k
                      )
        , true
        );

      b.erase (k);

      require_evaluating_to
        ( fmt::format ( "set_erase ({}, {}L)"
                      , pnet::type::value::show (a)
                      , k
                      )
        , b
        );

      a.erase (k);
    }

    require_evaluating_to
      ( fmt::format ("set_empty ({})", pnet::type::value::show (a))
      , true
      );
  }
}

BOOST_AUTO_TEST_CASE (tokens_set_top_pop)
{
  fhg::util::testing::random<long> number;

  for (int _ (0); _ < 10; ++_)
  {
    std::set<pnet::type::value::value_type> a;
    std::set<pnet::type::value::value_type> b;

    auto n (fhg::util::testing::random<unsigned int>{} (100));

    while (n --> 0)
    {
      long const k (number());

      a.insert (k);
      b.insert (k);

      require_evaluating_to
        ( fmt::format ("set_top ({})", pnet::type::value::show (a))
        , *(a.begin())
        );
    }

    while (!a.empty())
    {
      b.erase (b.begin());

      require_evaluating_to
        ( fmt::format ("set_pop ({})", pnet::type::value::show (a))
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

BOOST_AUTO_TEST_CASE
  (to_retrieve_non_existing_keys_from_empty_map_throws_informative_exception)
{
  std::map<pnet::type::value::value_type, pnet::type::value::value_type> m;

  auto const missing_key (fhg::util::testing::random<int>{}());

  fhg::util::testing::require_exception
    ( [&]
      {
        (void) expr::parse::parser
          ( fmt::format ( "map_get_assignment ({}, {})"
                        , pnet::type::value::show (m)
                        , pnet::type::value::show (missing_key)
                        )
          ).eval_all();
      }
    , pnet::exception::eval (expr::token::_map_get_assignment, m, missing_key)
    );
}

BOOST_AUTO_TEST_CASE
  (to_retrieve_non_existing_keys_from_non_empty_map_throws_informative_exception)
{
  struct identifier
  {
    std::string operator()() const
    {
      return fhg::util::testing::random_identifier();
    }
  };

  fhg::util::testing::unique_random<std::string, identifier> uniq_key;
  auto N (fhg::util::testing::random<std::size_t>{} (1000));

  std::map<pnet::type::value::value_type, pnet::type::value::value_type> m;

  auto random_int (fhg::util::testing::random<int>{});

  while (N --> 0)
  {
    m.emplace (uniq_key(), random_int());
  }

  auto const missing_key (uniq_key());

  fhg::util::testing::require_exception
    ( [&]
      {
        (void) expr::parse::parser
          ( fmt::format ( "map_get_assignment ({}, {})"
                        , pnet::type::value::show (m)
                        , pnet::type::value::show (missing_key)
                        )
          ).eval_all();
      }
    , pnet::exception::eval (expr::token::_map_get_assignment, m, missing_key)
    );
}

//! \todo add more tests

BOOST_AUTO_TEST_CASE (is_const_true)
{
  BOOST_REQUIRE_EQUAL (true, expr::parse::parser ("true").is_const_true());
  BOOST_REQUIRE_EQUAL (false, expr::parse::parser ("false").is_const_true());

  //! \todo make this evaluate to "true"
  BOOST_REQUIRE_EQUAL
    (false, expr::parse::parser ("${a} :eq: ${a}").is_const_true());

  BOOST_REQUIRE_EQUAL (false, expr::parse::parser ("${a}").is_const_true());
}

BOOST_AUTO_TEST_CASE (to_eval_stack_top_of_empty_list_throws_eval_error)
{
  fhg::util::testing::require_exception
    ( []
      {
        expr::parse::parser ("stack_top (List())").eval_all();
      }
    , pnet::exception::eval
      ( expr::token::_stack_top
      , std::list<pnet::type::value::value_type>{}
      )
    );
}
BOOST_AUTO_TEST_CASE (to_eval_stack_pop_of_empty_list_throws_eval_error)
{
  fhg::util::testing::require_exception
    ( []
      {
        expr::parse::parser ("stack_pop (List())").eval_all();
      }
    , pnet::exception::eval
      ( expr::token::_stack_pop
      , std::list<pnet::type::value::value_type>{}
      )
    );
}
BOOST_AUTO_TEST_CASE (to_eval_set_top_of_empty_set_throws_eval_error)
{
  fhg::util::testing::require_exception
    ( []
      {
        expr::parse::parser ("set_top (Set{})").eval_all();
      }
    , pnet::exception::eval
      ( expr::token::_set_top
      , std::set<pnet::type::value::value_type>{}
      )
    );
}
BOOST_AUTO_TEST_CASE (to_eval_set_pop_of_empty_set_throws_eval_error)
{
  fhg::util::testing::require_exception
    ( []
      {
        expr::parse::parser ("set_pop (Set{})").eval_all();
      }
    , pnet::exception::eval
      ( expr::token::_set_pop
      , std::set<pnet::type::value::value_type>{}
      )
    );
}

#define REQUIRE_NODE_IS_VALUE(n,v)                                        \
  fhg::util::visit<void>                                                  \
    ( n                                                                   \
    , [] (pnet::type::value::value_type const& value)                     \
      {                                                                   \
        BOOST_REQUIRE_EQUAL (value, pnet::type::value::value_type (v));   \
      }                                                                   \
    , [] (auto const& node)                                               \
      {                                                                   \
        BOOST_FAIL                                                        \
          (  "Expected value " << v << " but got node "                   \
          << expr::parse::node::type (node)                               \
          );                                                              \
      }                                                                   \
    )

BOOST_AUTO_TEST_CASE (constants_are_folded_by_default)
{
  expr::parse::parser const p ("4 + 5");

  BOOST_REQUIRE (p.begin() !=  p.end());
  BOOST_REQUIRE (std::next (p.begin()) == p.end());
  REQUIRE_NODE_IS_VALUE (*p.begin(), 9);
}
BOOST_AUTO_TEST_CASE (constant_folding_can_be_disabled)
{
  expr::parse::parser const p
    (expr::parse::parser::DisableConstantFolding{}, "4 + 5");

  BOOST_REQUIRE (p.begin() !=  p.end());
  BOOST_REQUIRE (std::next (p.begin()) == p.end());

  fhg::util::visit<void>
    ( *p.begin()
    , [] (expr::parse::node::binary_t const& b)
      {
        BOOST_REQUIRE_EQUAL (b.token, ::expr::token::add);
        REQUIRE_NODE_IS_VALUE (b.l, 4);
        REQUIRE_NODE_IS_VALUE (b.r, 5);
      }
    , [] (auto const& node)
      {
        BOOST_FAIL
          ("Expected '+' but got node " << expr::parse::node::type (node));
      }
    );
}
