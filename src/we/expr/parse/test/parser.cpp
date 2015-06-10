#define BOOST_TEST_MODULE we_expr_parse_parser
#include <boost/test/unit_test.hpp>

#include <we/expr/parse/parser.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

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
