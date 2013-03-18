// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE we_expr_literal_show

#include <we/type/literal/show.hpp>

#include <we/expr/parse/parser.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (preseve)
{
  const std::string input
    ( "(${x} := []);"
      "(${x} := true);"
      "(${x} := false);"
      "(${x} := 0L);"
      "(${x} := 0.00000);"
      "(${x} := 'c');"
      "(${x} := \"s\");"
      "(${x} := {});"
      "(${x} := @@);"
      "(${x} := {||});"
      "(${x} := {::});"
      "(${x} := y());"
    );

  const expr::parse::parser parser (input);

  BOOST_REQUIRE_EQUAL (parser.string(), input);
}
