// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_parser
#include <boost/test/unit_test.hpp>

#include <we/exception.hpp>
#include <we/expr/eval/context.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/type/value/boost/test/printer.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include <limits>
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

BOOST_AUTO_TEST_CASE (round_switches_between_half_up_and_half_down)
{
  require_evaluating_to ("round (2.5)", 2.0);
  require_evaluating_to ("round (2.5)", 3.0);
  require_evaluating_to ("round (2.5)", 2.0);
  require_evaluating_to ("round (2.5)", 3.0);
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

BOOST_AUTO_TEST_CASE (token_or_table)
{
  require_evaluating_to ("true || true", true);
  require_evaluating_to ("true || false", true);
  require_evaluating_to ("false || true", true);
  require_evaluating_to ("false || false", false);

  require_evaluating_to ("true :or: true", true);
  require_evaluating_to ("true :or: false", true);
  require_evaluating_to ("false :or: true", true);
  require_evaluating_to ("false :or: false", false);

  require_evaluating_to ("0 || 0", 0);
  require_evaluating_to ("0 || 1", 1);
  require_evaluating_to ("1 || 1", 1);
  require_evaluating_to ("1 || 0", 1);
  require_evaluating_to ("1 || 2", 3);
  require_evaluating_to ("2 || 1", 3);

  require_evaluating_to ("0U || 0U", 0U);
  require_evaluating_to ("0U || 1U", 1U);
  require_evaluating_to ("1U || 1U", 1U);
  require_evaluating_to ("1U || 0U", 1U);
  require_evaluating_to ("1U || 2U", 3U);
  require_evaluating_to ("2U || 1U", 3U);

  require_evaluating_to ("0L || 0L", 0L);
  require_evaluating_to ("0L || 1L", 1L);
  require_evaluating_to ("1L || 1L", 1L);
  require_evaluating_to ("1L || 0L", 1L);
  require_evaluating_to ("1L || 2L", 3L);
  require_evaluating_to ("2L || 1L", 3L);

  require_evaluating_to ("0UL || 0UL", 0UL);
  require_evaluating_to ("0UL || 1UL", 1UL);
  require_evaluating_to ("1UL || 1UL", 1UL);
  require_evaluating_to ("1UL || 0UL", 1UL);
  require_evaluating_to ("1UL || 2UL", 3UL);
  require_evaluating_to ("2UL || 1UL", 3UL);
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
                  , require_evaluating_to ("true || (${a} := true)", true)
                  , "a"
                  )
    , "missing binding for: ${a}"
    );

  BOOST_REQUIRE_EQUAL
    ( require_evaluating_to ("false || (${a} := true)", true).value ("a")
    , pnet::type::value::value_type (true)
    );
}

BOOST_AUTO_TEST_CASE (token_and_table)
{
  require_evaluating_to ("true && true", true);
  require_evaluating_to ("true && false", false);
  require_evaluating_to ("false && true", false);
  require_evaluating_to ("false && false", false);

  require_evaluating_to ("true :and: true", true);
  require_evaluating_to ("true :and: false", false);
  require_evaluating_to ("false :and: true", false);
  require_evaluating_to ("false :and: false", false);

  require_evaluating_to ("0 && 0", 0);
  require_evaluating_to ("0 && 1", 0);
  require_evaluating_to ("1 && 1", 1);
  require_evaluating_to ("1 && 0", 0);
  require_evaluating_to ("1 && 2", 0);
  require_evaluating_to ("2 && 1", 0);
  require_evaluating_to ("2 && 3", 2);

  require_evaluating_to ("0U && 0U", 0U);
  require_evaluating_to ("0U && 1U", 0U);
  require_evaluating_to ("1U && 1U", 1U);
  require_evaluating_to ("1U && 0U", 0U);
  require_evaluating_to ("1U && 2U", 0U);
  require_evaluating_to ("2U && 1U", 0U);
  require_evaluating_to ("2U && 3U", 2U);

  require_evaluating_to ("0L && 0L", 0L);
  require_evaluating_to ("0L && 1L", 0L);
  require_evaluating_to ("1L && 1L", 1L);
  require_evaluating_to ("1L && 0L", 0L);
  require_evaluating_to ("1L && 2L", 0L);
  require_evaluating_to ("2L && 1L", 0L);
  require_evaluating_to ("2L && 3L", 2L);

  require_evaluating_to ("0UL && 0UL", 0UL);
  require_evaluating_to ("0UL && 1UL", 0UL);
  require_evaluating_to ("1UL && 1UL", 1UL);
  require_evaluating_to ("1UL && 0UL", 0UL);
  require_evaluating_to ("1UL && 2UL", 0UL);
  require_evaluating_to ("2UL && 1UL", 0UL);
  require_evaluating_to ("2UL && 3UL", 2UL);
}

BOOST_AUTO_TEST_CASE (token_and_short_circuit)
{
  fhg::util::boost::test::require_exception<pnet::exception::missing_binding>
    ( boost::bind ( &get_and_ignore_value_from_context
                  , require_evaluating_to ("false && (${a} := false)", false)
                  , "a"
                  )
    , "missing binding for: ${a}"
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
  require_evaluating_to ("!0", true);
  require_evaluating_to ("!1", false);
  require_evaluating_to ("!0U", true);
  require_evaluating_to ("!1U", false);
  require_evaluating_to ("!0L", true);
  require_evaluating_to ("!1L", false);
  require_evaluating_to ("!0UL", true);
  require_evaluating_to ("!1UL", false);
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
  void require_random_integrals_evaluating_to
    (boost::function<void (T const&, T const&)> check)
  {
    boost::random::random_device generator;
    boost::random::uniform_int_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 1000; ++i)
    {
      check (number (generator), number (generator));
    }
  }

  template<typename T>
  void check_integral ( std::string const& operation_string
                      , boost::function<T (T const&, T const&)> operation
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
  void check_fractional ( std::string const& operation_string
                        , boost::function<T (T const&, T const&)> operation
                        )
  {
    boost::random::random_device generator;
    //! \todo possible fix ::min to something else
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
  require_evaluating_to ("'a' + 'a'", std::string ("aa"));
  require_evaluating_to ("\"\" + \"\"", std::string (""));
  require_evaluating_to ("\"a\" + \"\"", std::string ("a"));
  require_evaluating_to ("\"a\" + \"a\"", std::string ("aa"));
  require_evaluating_to ("\"ab\" + \"a\"", std::string ("aba"));

  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_integral<int>, "+", &plus<int>, _1, _2));
  require_random_integrals_evaluating_to<unsigned int>
    (boost::bind (&check_integral<unsigned int>, "+", &plus<unsigned int>, _1, _2));
  require_random_integrals_evaluating_to<long>
    (boost::bind (&check_integral<long>, "+", &plus<long>, _1, _2));
  require_random_integrals_evaluating_to<unsigned long>
    (boost::bind (&check_integral<unsigned long>, "+", &plus<unsigned long>, _1, _2));

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

  check_fractional<float> ("+", &plus<float>);
  check_fractional<double> ("+", &plus<double>);

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
    (boost::bind (&check_integral<int>, "*", &product<int>, _1, _2));
  require_random_integrals_evaluating_to<unsigned int>
    (boost::bind (&check_integral<unsigned int>, "*", &product<unsigned int>, _1, _2));
  require_random_integrals_evaluating_to<long>
    (boost::bind (&check_integral<long>, "*", &product<long>, _1, _2));
  require_random_integrals_evaluating_to<unsigned long>
    (boost::bind (&check_integral<unsigned long>, "*", &product<unsigned long>, _1, _2));

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

  check_fractional<float> ("*", &product<float>);
  check_fractional<double> ("*", &product<double>);

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
      fhg::util::boost::test::require_exception<std::runtime_error>
        ( boost::bind (&parser_ctor, expression)
        , "r > l => neg result"
        );
    }
  }
}

BOOST_AUTO_TEST_CASE (token_sub)
{
  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_integral<int>, "-", &minus<int>, _1, _2));
  require_random_integrals_evaluating_to<long>
    (boost::bind (&check_integral<long>, "-", &minus<long>, _1, _2));

  require_evaluating_to ("0 - 0", 0);
  require_evaluating_to ("1 - 0", 1);
  require_evaluating_to ("0 - 1", -1);
  require_evaluating_to ("0L - 0L", 0L);
  require_evaluating_to ("1L - 0L", 1L);
  require_evaluating_to ("0L - 1L", -1L);

  check_fractional<float> ("-", &minus<float>);
  check_fractional<double> ("-", &minus<double>);

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

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0U - 1U")
    , "r > l => neg result"
    );

  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&parser_ctor, "0UL - 1UL")
    , "r > l => neg result"
    );
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
    , boost::function<T (T const&, T const&)> operation
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
      fhg::util::boost::test::require_exception
        <expr::exception::eval::divide_by_zero>
        (boost::bind (&parser_ctor, expression), "divide by zero");
    }
  }
}

BOOST_AUTO_TEST_CASE (token_divint)
{
  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_divmod_for_integral<int>, "div", &quotient<int>, _1, _2));
  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_divmod_for_integral<unsigned int>, "div", &quotient<unsigned int>, _1, _2));
  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_divmod_for_integral<long>, "div", &quotient<long>, _1, _2));
  require_random_integrals_evaluating_to<int>
    (boost::bind (&check_divmod_for_integral<unsigned long>, "div", &quotient<unsigned long>, _1, _2));

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

  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1 div 0"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1U div 0U"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1L div 0L"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1UL div 0UL"), "divide by zero");
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
    (boost::bind (&check_divmod_for_integral<int>, "mod", &remainder<int>, _1, _2));
  require_random_integrals_evaluating_to<unsigned int>
    (boost::bind (&check_divmod_for_integral<unsigned int>, "mod", &remainder<unsigned int>, _1, _2));
  require_random_integrals_evaluating_to<long>
    (boost::bind (&check_divmod_for_integral<long>, "mod", &remainder<long>, _1, _2));
  require_random_integrals_evaluating_to<unsigned long>
    (boost::bind (&check_divmod_for_integral<unsigned long>, "mod", &remainder<unsigned long>, _1, _2));

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

  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1 mod 0"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1U mod 0U"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1L mod 0L"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1UL mod 0UL"), "divide by zero");
}

namespace
{
  template<typename T>
  void check_quotient_for_fractional()
  {
    boost::random::random_device generator;
    //! \todo possible fix ::min to something else
    boost::random::uniform_real_distribution<T> number
      (std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

    for (int i (0); i < 1000; ++i)
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
        fhg::util::boost::test::require_exception
          <expr::exception::eval::divide_by_zero>
          (boost::bind (&parser_ctor, expression), "divide by zero");
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

  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1.0 / 0.0"), "divide by zero");
  fhg::util::boost::test::require_exception
    <expr::exception::eval::divide_by_zero>
    (boost::bind (&parser_ctor, "1.0f / 0.0f"), "divide by zero");
}
