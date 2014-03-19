// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser
#include <boost/test/unit_test.hpp>

#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>

#include <we/exception.hpp>

#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <string>
#include <limits>

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
  expr::eval::context check (std::string const& expression, T const& value)
  {
    expr::eval::context context;

    BOOST_REQUIRE_EQUAL ( expr::parse::parser (expression).eval_front (context)
                        , pnet::type::value::value_type (value)
                        );

    return context;
  }
}

BOOST_AUTO_TEST_CASE (round_switches_between_half_up_and_half_down)
{
  check ("round (2.5)", 2.0);
  check ("round (2.5)", 3.0);
  check ("round (2.5)", 2.0);
  check ("round (2.5)", 3.0);
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
  check ("ceil (0.0)", 0.0);
  check ("ceil (0.25)", 1.0);
  check ("ceil (0.5)", 1.0);
  check ("ceil (0.75)", 1.0);
  check ("ceil (1.0)", 1.0);
  check ("ceil (1.25)", 2.0);
  check ("ceil (1.5)", 2.0);
  check ("ceil (1.75)", 2.0);
  check ("ceil (2.0)", 2.0);
}

BOOST_AUTO_TEST_CASE (_floor)
{
  check ("floor (0.0)", 0.0);
  check ("floor (0.25)", 0.0);
  check ("floor (0.5)", 0.0);
  check ("floor (0.75)", 0.0);
  check ("floor (1.0)", 1.0);
  check ("floor (1.25)", 1.0);
  check ("floor (1.5)", 1.0);
  check ("floor (1.75)", 1.0);
  check ("floor (2.0)", 2.0);
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
  check ("true || true", true);
  check ("true || false", true);
  check ("false || true", true);
  check ("false || false", false);

  check ("true :or: true", true);
  check ("true :or: false", true);
  check ("false :or: true", true);
  check ("false :or: false", false);

  check ("0 || 0", 0);
  check ("0 || 1", 1);
  check ("1 || 1", 1);
  check ("1 || 0", 1);
  check ("1 || 2", 3);
  check ("2 || 1", 3);

  check ("0U || 0U", 0U);
  check ("0U || 1U", 1U);
  check ("1U || 1U", 1U);
  check ("1U || 0U", 1U);
  check ("1U || 2U", 3U);
  check ("2U || 1U", 3U);

  check ("0L || 0L", 0L);
  check ("0L || 1L", 1L);
  check ("1L || 1L", 1L);
  check ("1L || 0L", 1L);
  check ("1L || 2L", 3L);
  check ("2L || 1L", 3L);

  check ("0UL || 0UL", 0UL);
  check ("0UL || 1UL", 1UL);
  check ("1UL || 1UL", 1UL);
  check ("1UL || 0UL", 1UL);
  check ("1UL || 2UL", 3UL);
  check ("2UL || 1UL", 3UL);
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
  fhg::util::boost::test::require_exception<pnet::exception::missing_binding>
    ( boost::bind ( &get_and_ignore_value_from_context
                  , check ("true || (${a} := true)", true)
                  , "a"
                  )
    , "missing binding for: ${a}"
    );

  BOOST_REQUIRE_EQUAL
    ( check ("false || (${a} := true)", true).value ("a")
    , pnet::type::value::value_type (true)
    );
}

BOOST_AUTO_TEST_CASE (token_and_table)
{
  check ("true && true", true);
  check ("true && false", false);
  check ("false && true", false);
  check ("false && false", false);

  check ("true :and: true", true);
  check ("true :and: false", false);
  check ("false :and: true", false);
  check ("false :and: false", false);

  check ("0 && 0", 0);
  check ("0 && 1", 0);
  check ("1 && 1", 1);
  check ("1 && 0", 0);
  check ("1 && 2", 0);
  check ("2 && 1", 0);

  check ("0U && 0U", 0U);
  check ("0U && 1U", 0U);
  check ("1U && 1U", 1U);
  check ("1U && 0U", 0U);
  check ("1U && 2U", 0U);
  check ("2U && 1U", 0U);

  check ("0L && 0L", 0L);
  check ("0L && 1L", 0L);
  check ("1L && 1L", 1L);
  check ("1L && 0L", 0L);
  check ("1L && 2L", 0L);
  check ("2L && 1L", 0L);

  check ("0UL && 0UL", 0UL);
  check ("0UL && 1UL", 0UL);
  check ("1UL && 1UL", 1UL);
  check ("1UL && 0UL", 0UL);
  check ("1UL && 2UL", 0UL);
  check ("2UL && 1UL", 0UL);
}

BOOST_AUTO_TEST_CASE (token_and_short_circuit)
{
  fhg::util::boost::test::require_exception<pnet::exception::missing_binding>
    ( boost::bind ( &get_and_ignore_value_from_context
                  , check ("false && (${a} := false)", false)
                  , "a"
                  )
    , "missing binding for: ${a}"
    );

  BOOST_REQUIRE_EQUAL
    ( check ("true && (${a} := false)", false).value ("a")
    , pnet::type::value::value_type (false)
    );
}

BOOST_AUTO_TEST_CASE (token_not)
{
  check ("!true", false);
  check ("!false", true);
  check ("!!true", true);
  check ("!!false", false);
  check ("!0", true);
  check ("!1", false);
  check ("!0U", true);
  check ("!1U", false);
  check ("!0L", true);
  check ("!1L", false);
  check ("!0UL", true);
  check ("!1UL", false);
}

namespace
{
  void check_compare
    (std::string lhs, std::string rhs, bool lt, bool le, bool gt, bool ge)
  {
    check ((boost::format ("%1% < %2%") % lhs % rhs).str(), lt);
    check ((boost::format ("%1% <= %2%") % lhs % rhs).str(), le);
    check ((boost::format ("%1% > %2%") % lhs % rhs).str(), gt);
    check ((boost::format ("%1% >= %2%") % lhs % rhs).str(), ge);

    check ((boost::format ("%1% :lt: %2%") % lhs % rhs).str(), lt);
    check ((boost::format ("%1% :le: %2%") % lhs % rhs).str(), le);
    check ((boost::format ("%1% :gt: %2%") % lhs % rhs).str(), gt);
    check ((boost::format ("%1% :ge: %2%") % lhs % rhs).str(), ge);
  }

  void check_equality (std::string lhs, std::string rhs, bool eq)
  {
    check ((boost::format ("%1% != %2%") % lhs % rhs).str(), !eq);
    check ((boost::format ("%1% == %2%") % lhs % rhs).str(), eq);

    check ((boost::format ("%1% :ne: %2%") % lhs % rhs).str(), !eq);
    check ((boost::format ("%1% :eq: %2%") % lhs % rhs).str(), eq);
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
  check_equality ("{}", "bitset_insert {} 1L", false);
  check_equality ("bitset_insert {} 1L", "{}", false);
  check_equality ("bitset_insert {} 1L", "bitset_insert {} 2L", false);
  check_equality ( "bitset_insert (bitset_insert {} 1L) 2L"
                 , "bitset_insert {} 2L", false
                 );
  check_equality ( "bitset_insert (bitset_insert {} 1L) 2L"
                 , "bitset_insert (bitset_insert {} 2L) 1L", true
                 );

  check_equality ("y()", "y()", true);
  check_equality ("y(4)", "y()", false);
  check_equality ("y()", "y(4)", false);
  check_equality ("y(4)", "y(4)", true);
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
  void check_integral ( std::string const& operation_string
                      , boost::function<T (T const&, T const&)> operation
                      )
  {
    boost::random::random_device generator;
    boost::random::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 1000; ++i)
    {
      T const l (number (generator));
      T const r (number (generator));

      check
        ( ( boost::format ("%1%%3% %4% %2%%3%")
          % l
          % r
          % suffix<T>()()
          % operation_string
          ).str()
        , operation (l, r)
        );
    }
  }

  template<typename T>
  void check_fractional ( std::string const& operation_string
                        , boost::function<T (T const&, T const&)> operation
                        )
  {
    boost::random::random_device generator;
    boost::random::uniform_real_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 1000; ++i)
    {
      std::string const l
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());
      std::string const r
        ((boost::format ("%1%%2%") % number (generator) % suffix<T>()()).str());

      expr::eval::context context;

      BOOST_REQUIRE_EQUAL
        ( boost::get<T>
          ( expr::parse::parser
            ((boost::format ("%1% %3% %2%") % l % r % operation_string).str())
          . eval_front (context)
          )
        , operation
          ( boost::get<T> (expr::parse::parser (l).eval_front (context))
          , boost::get<T> (expr::parse::parser (r).eval_front (context))
          )
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
  check ("'a' + 'a'", std::string ("aa"));
  check ("\"\" + \"\"", std::string (""));
  check ("\"a\" + \"\"", std::string ("a"));
  check ("\"a\" + \"a\"", std::string ("aa"));
  check ("\"ab\" + \"a\"", std::string ("aba"));

  check_integral<int> ("+", &plus<int>);
  check_integral<unsigned int> ("+", &plus<unsigned int>);
  check_integral<long> ("+", &plus<long>);
  check_integral<unsigned long> ("+", &plus<unsigned long>);

  check ("0 + 0", 0);
  check ("0 + 1", 1);
  check ("1 + 0", 1);
  check ("0U + 0U", 0U);
  check ("0U + 1U", 1U);
  check ("1U + 0U", 1U);
  check ("0L + 0L", 0L);
  check ("0L + 1L", 1L);
  check ("1L + 0L", 1L);
  check ("0UL + 0UL", 0UL);
  check ("0UL + 1UL", 1UL);
  check ("1UL + 0UL", 1UL);

  check_fractional<float> ("+", &plus<float>);
  check_fractional<double> ("+", &plus<double>);

  check ("0.0 + 0.0", 0.0);
  check ("0.0 + 1.0", 1.0);
  check ("1.0 + 0.0", 1.0);
  check ("1.0 + 1.0", 2.0);
  check ("0.0f + 0.0f", 0.0f);
  check ("0.0f + 1.0f", 1.0f);
  check ("1.0f + 0.0f", 1.0f);
  check ("1.0f + 1.0f", 2.0f);
}

namespace
{
  void parser_ctor (std::string const& input)
  {
    (void)expr::parse::parser (input);
  }

  template<typename T>
    T minus (T const& l, T const& r)
  {
    return l - r;
  }
}

namespace
{
  template<typename T>
  void check_minus_for_unsigned_integral
    ( std::string const& operation_string
    , boost::function<T (T const&, T const&)> operation
    )
  {
    boost::random::random_device generator;
    boost::random::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 1000; ++i)
    {
      T const l (number (generator));
      T const r (number (generator));

      std::string const expression
        ( ( boost::format ("%1%%3% %4% %2%%3%")
          % l
          % r
          % suffix<T>()()
          % operation_string
          ).str()
        );

      if (l >= r)
      {
        check (expression, operation (l, r));
      }
      else
      {
        fhg::util::boost::test::require_exception<std::runtime_error>
          ( boost::bind (&parser_ctor, expression)
          , "r > l => neg result"
          );
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (token_sub)
{
  check_integral<int> ("-", &minus<int>);
  check_integral<long> ("-", &minus<long>);

  check ("0 - 0", 0);
  check ("1 - 0", 1);
  check ("0 - 1", -1);
  check ("0L - 0L", 0L);
  check ("1L - 0L", 1L);
  check ("0L - 1L", -1L);

  check_fractional<float> ("-", &minus<float>);
  check_fractional<double> ("-", &minus<double>);

  check ("0.0 - 0.0", 0.0);
  check ("0.0 - 1.0", -1.0);
  check ("1.0 - 1.0", 0.0);
  check ("0.0f - 0.0f", 0.0f);
  check ("0.0f - 1.0f", -1.0f);
  check ("1.0f - 1.0f", 0.0f);

  check_minus_for_unsigned_integral<unsigned int> ("-", &minus<unsigned int>);
  check_minus_for_unsigned_integral<unsigned long> ("-", &minus<unsigned long>);

  check ("0U - 0U", 0U);
  check ("1U - 0U", 1U);
  check ("2U - 1U", 1U);
  check ("0UL - 0UL", 0UL);
  check ("1UL - 0UL", 1UL);
  check ("2UL - 1UL", 1UL);

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0U - 1U")
    , "r > l => neg result"
    );

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0UL - 1UL")
    , "r > l => neg result"
    );
}
